/**
 * LOC File Compiler
 */
#include <LOCC.h>
#include <argh.h>

#include <BM/LOC/LOCTypes.h>
#include <BM/LOC/LOCJson.h>
#include <BM/LOC/LOCTree.h>

struct ResourceCollection
{
public:
    static char* Lookup(char* key, char* buffer);
};

struct FIO
{
    static bool HasFile(std::string_view path);

    static std::unique_ptr<char[]> ReadFile(std::string_view path, size_t& bufferSize);
    static nlohmann::json ReadJson(std::string_view path);

    static bool WriteFile(std::string_view path, const char* buffer, size_t bufferSize);
    static bool WriteFile(std::string_view path, const nlohmann::json& json, int indent = -1);
};

enum CompilerRetCodes : int {
    // Success
    Ok = 0,
    // Errors
    FailedToOpenSource = -10,
    FailedToOpenDestination = -11,
    FailedToReadSource = -12,
    BadSourceFile = -13,
    FailedToCompileTree = -14,
    FailedToWriteDestination = -15,
    TestFailed = -16
};

struct CompilerOptions_t
{
    struct Consts
    {
        static constexpr const char* kNoSource = "<NO_SOURCE>";
    };

    std::string From;
    std::string To;
    std::string DecompileTo;
    bool TestFinalFile { false };
    bool TestIgnoreErrors { false };
    bool ShowHelp { false };
} g_compilerOptions {};

int ShowHelp()
{
    spdlog::info("LOC Compiler tool help: ");
    spdlog::info("Options: ");
    spdlog::info("\t--help - Show this help");
    spdlog::info("\t--from - Path to JSON file (from GMSInfo tool) | Required");
    spdlog::info("\t--to - Path to final LOC file | Required");
    spdlog::info("\t--test - Check final file by iterate by each key and try to find it | Default: false");
    spdlog::info("\t--test-ignore-errors - Ignore errors in testing | Default: false");
    spdlog::info("\t--decompile-json - Path to json for decompiler from file (override --to option!)");
    spdlog::info("Usage: ");
    spdlog::info("\tlocc.exe --from=(path to file) --to=(path to file) --test=1");
    return 0;
}

int main(int argc, char** argv)
{
    argh::parser cli(argc, argv);

    cli("--help", g_compilerOptions.ShowHelp) >> g_compilerOptions.ShowHelp;
    cli("--from", CompilerOptions_t::Consts::kNoSource) >> g_compilerOptions.From;
    cli("--to", CompilerOptions_t::Consts::kNoSource) >> g_compilerOptions.To;
    cli("--test", g_compilerOptions.TestFinalFile) >> g_compilerOptions.TestFinalFile;
    cli("--test-ignore-errors", g_compilerOptions.TestIgnoreErrors) >> g_compilerOptions.TestIgnoreErrors;
    cli("--decompile-json", g_compilerOptions.DecompileTo) >> g_compilerOptions.DecompileTo;

    if (g_compilerOptions.ShowHelp) return ShowHelp();

    if (g_compilerOptions.From.empty())
    {
        spdlog::error("--from option required!");
        return ShowHelp();
    }

    if (g_compilerOptions.To.empty() && g_compilerOptions.DecompileTo.empty())
    {
        spdlog::error("--to or --decompile-json required!");
        return ShowHelp();
    }

    if (!FIO::HasFile(g_compilerOptions.From))
    {
        spdlog::error("LOCC::Error: Failed to open source file {}", g_compilerOptions.From);
        return CompilerRetCodes::FailedToOpenSource;
    }

    CompilerRetCodes returnCode = CompilerRetCodes::Ok;

    if (!g_compilerOptions.DecompileTo.empty())
    {
        spdlog::info("LOCC: Decompile file {} to {}", g_compilerOptions.From, g_compilerOptions.DecompileTo);
        // Decompiler mode

        size_t bufferSize = 0;
        auto buffer = FIO::ReadFile(g_compilerOptions.From, bufferSize);
        if (!buffer)
        {
            spdlog::error("LOCC: Failed to read source file {}", g_compilerOptions.From);
            return CompilerRetCodes::BadSourceFile;
        }

        // Run decompiler
        auto root = BM::LOC::LOCTreeNode::ReadFromMemory(buffer.get(), bufferSize);
        if (!root)
        {
            spdlog::error("Failed to decompile LOC file {}", g_compilerOptions.From);
            return CompilerRetCodes::BadSourceFile;
        }

        nlohmann::json j;
        try
        {
            nlohmann::adl_serializer<BM::LOC::LOCTreeNode>::to_json(j, root);
        } catch (const std::exception& ex)
        {
            spdlog::error("LOCC: Failed to serialize passed tree! Error: {}", ex.what());
            delete root;
            return CompilerRetCodes::BadSourceFile;
        }

        delete root;

        if (!FIO::WriteFile(g_compilerOptions.DecompileTo, j))
        {
            spdlog::error("LOCC: Failed to write output file {}", g_compilerOptions.DecompileTo);
            return CompilerRetCodes::FailedToWriteDestination;
        }

        spdlog::info("LOCC: File {} decompiled to {} successfully!", g_compilerOptions.From, g_compilerOptions.DecompileTo);
        returnCode = CompilerRetCodes::Ok;
    }
    else
    {
        spdlog::info("LOCC: Compile file {} to {}", g_compilerOptions.From, g_compilerOptions.To);
        nlohmann::json localizationInJson = FIO::ReadJson(g_compilerOptions.From);
        if (localizationInJson.empty())
        {
            spdlog::error("LOCC: Failed to read source localization json {}", g_compilerOptions.From);
            return CompilerRetCodes::BadSourceFile;
        }

        auto root = new BM::LOC::LOCTreeNode(nullptr, nullptr); // No parent, no buffer
        try
        {
            nlohmann::adl_serializer<BM::LOC::LOCTreeNode>::from_json(localizationInJson, root);
        }
        catch (const nlohmann::json::exception& badJson)
        {
            spdlog::error("LOCC::Error: Bad source json! Message: {}", badJson.what());
            return CompilerRetCodes::BadSourceFile;
        }
        catch (const std::exception& formatError)
        {
            spdlog::error("LOCC::Error: Bad format! Reason: {}", formatError.what());
            return CompilerRetCodes::BadSourceFile;
        }

        bool isCompiled = false;
        bool isWritten = false;

        std::vector<uint8_t> compiledBuffer {};
        BM::LOC::LOCTreeNode::CacheDataBase cacheDataBase {};

        try {
            if (!BM::LOC::LOCTreeNode::Compile(root, compiledBuffer))
            {
                isCompiled = false;
            }
            else
            {
                isCompiled = true;

                if (g_compilerOptions.TestFinalFile)
                {
                    spdlog::info("LLOC| Generating cache ...");
                    BM::LOC::LOCTreeNode::GenerateCacheDataBase(root, cacheDataBase);
                    spdlog::info("LLOC| Cache generated! Entries count: {}", cacheDataBase.size());
                }

                if (!FIO::WriteFile(g_compilerOptions.To, (char*)compiledBuffer.data(), compiledBuffer.size()))
                {
                    spdlog::error("LOCC::Error: Failed to write compiled tree into file {}!", g_compilerOptions.To);
                    returnCode = CompilerRetCodes::FailedToWriteDestination;
                }
                else
                {
                    spdlog::info(" Compile success! Output file: {}", g_compilerOptions.To);
                    returnCode = CompilerRetCodes::Ok;
                    isWritten = true;
                }
            }
        }
        catch (const std::exception& compileError)
        {
            spdlog::error("LOCC::Error: Failed to compile tree. Reason: {}", compileError.what());
            returnCode = CompilerRetCodes::FailedToCompileTree;
        }

        delete root;
        root = nullptr;

        if (isCompiled && isWritten)
        {
            if (g_compilerOptions.TestFinalFile)
            {
                if (cacheDataBase.empty())
                {
                    spdlog::error("LOCC::Error: Unable to test the final file. No cache database generated. Possible bug!");
                }
                else
                {
                    spdlog::info("LLOC: Start testing of compiled LOC");
                    size_t bufferSize = 0;
                    auto buffer = FIO::ReadFile(g_compilerOptions.To, bufferSize);
                    if (!buffer)
                    {
                        spdlog::error("LLOC: Test failed! Failed to read file {}", g_compilerOptions.To);
                        return CompilerRetCodes::FailedToOpenDestination;
                    }

                    struct TestStats
                    {
                        int success   = 0;
                        int failed    = 0;
                        int not_found = 0;
                    };

                    TestStats stats {};

                    int keyIndex = 1;
                    bool isTestPassed = true;
                    for (const auto& [path, requiredValue] : cacheDataBase)
                    {
                        auto result = ResourceCollection::Lookup((char*)path.data(), buffer.get());
                        if (!result)
                        {
                            spdlog::error("LLOC: [{:4d}] Key '{}' - NOT FOUND", keyIndex, path);
                            ++stats.failed;
                            ++stats.not_found;
                            isTestPassed = false;
                            if (!g_compilerOptions.TestIgnoreErrors)
                                break;
                        }
                        else
                        {
                            if (std::string(result + 1) != requiredValue)
                            {
                                spdlog::error("LLOC: [{:4d}] Key '{}' FOUND, WRONG RESULT. Expected '{}', received '{}'", keyIndex, path, requiredValue, (result + 1));
                                isTestPassed = false;
                                ++stats.failed;
                                if (!g_compilerOptions.TestIgnoreErrors)
                                    break;
                            }
                            else
                            {
                                spdlog::info("LLOC: [{:4d}] Key '{}' PASSED", keyIndex, path);
                                ++stats.success;
                            }
                        }

                        ++keyIndex;
                    }

                    if (!isTestPassed)
                    {
                        spdlog::error("LLOC: Test failed!");
                        returnCode = CompilerRetCodes::TestFailed;
                    }
                    else
                    {
                        spdlog::info("LLOC: Test passed!");
                        returnCode = CompilerRetCodes::Ok;
                    }

                    spdlog::info("LLOC: Test stats:\n\tSuccess: {}\n\tFailed: {}\n\tNot Found: {}", stats.success, stats.failed, stats.not_found);
                }
            }
        }
    }

    return static_cast<int>(returnCode);
}

bool FIO::HasFile(std::string_view path)
{
    FILE* fp = fopen(path.data(), "rb");
    if (!fp) return false;
    fclose(fp);
    return true;
}

std::unique_ptr<char[]> FIO::ReadFile(std::string_view path, size_t& bufferSize)
{
    std::unique_ptr<char[]> result = nullptr;

    FILE* fp = fopen(path.data(), "rb");
    if (!fp)
    {
        spdlog::error("FIO::ReadFile| Failed to open file {} to read binary contents!", path);
        return nullptr;
    }

    // Recognize the size
    fseek(fp, 0L, SEEK_END);
    bufferSize = ftell(fp);
    rewind(fp);

    if (bufferSize == 0)
    {
        spdlog::warn("FIO::ReadFile| File {} is empty. Nothing to return/allocate", path);
        return nullptr; // Empty file - no buffer
    }

    result = std::make_unique<char[]>(bufferSize);
    auto readBytes = fread(result.get(), sizeof(char), bufferSize, fp);
    fclose(fp);

    if (readBytes != bufferSize)
    {
        spdlog::error("FIO::ReadFile| Failed to read file {}. Requested {} bytes, got {} bytes {}", path, bufferSize, readBytes);
        bufferSize = 0;
        return nullptr;
    }

    return result;
}

nlohmann::json FIO::ReadJson(std::string_view path)
{
    std::ifstream file { path.data(), std::ifstream::in };
    if (!file.good())
    {
        spdlog::error("FIO::ReadJson| Failed to read file {}", path);
        return nlohmann::json {};
    }

    try
    {
        nlohmann::json j;
        file >> j;
        file.close();
        return j;
    }catch (const nlohmann::json::exception& ex)
    {
        spdlog::error("FIO::ReadJson| Failed to parse json from file {}. Reason: {}", path, ex.what());
    }
    catch (const std::exception& STDERR)
    {
        spdlog::error("FIO::ReadJson| Failed to parse json from file {}. STD Reason: {}", path, STDERR.what());
    }

    return nlohmann::json {};
}

bool FIO::WriteFile(std::string_view path, const char* buffer, size_t bufferSize)
{
    FILE* fp = fopen(path.data(), "wb");
    if (!fp) {
        spdlog::error("FIO::WriteFile| Failed to open file {} to write binary contents", path);
        return false;
    }

    auto writtenBytes = fwrite(buffer, sizeof(char), bufferSize, fp);
    fclose(fp);

    if (writtenBytes != bufferSize)
    {
        spdlog::error("FIO::WriteFile| Failed to write buffer of size {} into file {}. Actually written {} bytes", path, bufferSize, writtenBytes);
        return false;
    }

    return true;
}

bool FIO::WriteFile(std::string_view path, const nlohmann::json& json, int indent)
{
    std::string buffer = json.dump(indent);
    return FIO::WriteFile(path, buffer.data(), buffer.size());
}

char* ResourceCollection::Lookup(char* key, char* buffer)
{
    // This is result of reverse engineering of function at 0x00464FF0 aka ResourceCollection::Lookup
    char *keyWithoutLeadingSlash; // edx
    char currentChar; // al
    char currentCharInKeyWithoutLeadingSlash; // al
    size_t newIndex; // ecx
    int numChild; // ebx
    int keyIndex; // ebp
    int v8; // edi
    int v9; // eax
    int v10; // eax
    int v11; // esi
    int v12; // esi
    char *valuePtr; // edx
    size_t index; // [esp+10h] [ebp-Ch]
    int numChildOrg; // [esp+14h] [ebp-8h]
    char *pChunkName; // [esp+18h] [ebp-4h]

    while (true)
    {
        /// KEY NORMALISATION
        keyWithoutLeadingSlash = key;
        if ( *key == '/' )  /// Search place where our key starts not from /
        {
            do
                currentChar = (keyWithoutLeadingSlash++)[1];
            while (currentChar == '/' );
            key = keyWithoutLeadingSlash;
        }

        currentCharInKeyWithoutLeadingSlash = *keyWithoutLeadingSlash;
        newIndex = 0;
        index = 0;

        if (*keyWithoutLeadingSlash != '/' )
        {
            do
            {
                if ( !currentCharInKeyWithoutLeadingSlash ) // If we have zero terminator -> break
                    break;

                currentCharInKeyWithoutLeadingSlash = keyWithoutLeadingSlash[newIndex++ + 1]; // save current char and increment newIndex
            }
            while (currentCharInKeyWithoutLeadingSlash != '/' ); // if our new char not slash -> continue

            index = newIndex;
        }

        /// KEY SEARCH AT THE CURRENT BRANCH
        numChild = (unsigned __int8)*buffer;
        keyIndex = 0;
        numChildOrg = (unsigned __int8)*buffer;
        if (numChild <= 0 )
            goto OnOrphanedTreeNodeDetected;

        pChunkName = &buffer[4 * numChild - 3];
        do
        {
            v8 = (numChild >> 1) + keyIndex;
            if ( v8 )
                v9 = *(int *)&buffer[4 * v8 - 3];
            else
                v9 = 0;

            int ret = 0;
            if ((ret = strnicmp(&pChunkName[v9], keyWithoutLeadingSlash, newIndex)) >= 0) // if value of first group greater or equal to our key
            {
                numChild >>= 1; // Divide by two
            }
            else // Group name is less than our key
            {
                keyIndex = v8 + 1;
                numChild += -1 - (numChild >> 1);
            }

            newIndex = index;
            keyWithoutLeadingSlash = key;
        }
        while (numChild > 0);

        /// VALUE RESOLVING
        numChild = numChildOrg; //Restore back? o_0
        if ( keyIndex )
            v10 = *(int*)&buffer[4 * keyIndex - 3];
        else
            OnOrphanedTreeNodeDetected:
            v10 = 0;
        v11 = v10 + 4 * numChild - 3;

        int ret = 0;
        if (keyIndex >= numChild || (ret = strnicmp(&buffer[v11], keyWithoutLeadingSlash, newIndex)))
        {
            return nullptr;
        }

        /// ITERATION OVER TREE
        v12 = index + v11;
        valuePtr = &buffer[v12 + 2];
        buffer += v12 + 2;

        if ( !key[index] )
        {
            return valuePtr - 1;
        }

        key += index + 1;
    }
}