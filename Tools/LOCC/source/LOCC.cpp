/**
 * LOC File Compiler
 */
#include <LOCC.h>
#include <argh.h>

#include <BM/LOC/LOCTypes.h>
#include <BM/LOC/LOCJson.h>
#include <BM/LOC/LOCTree.h>

enum CompilerRetCodes : int {
    // Success
    Ok = 0,
    // Errors
    FailedToOpenSource = -10,
    FailedToOpenDestination = -11,
    FailedToReadSource = -12,
    BadSourceFile = -13,
    FailedToCompileTree = -14,
    FailedToWriteDestination = -15
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
    cli("--decompile-to-json", g_compilerOptions.DecompileTo) >> g_compilerOptions.DecompileTo;

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

    std::ifstream inFile { g_compilerOptions.From, std::ios::in };
    if (!inFile)
    {
        spdlog::error("LOCC::Error: Failed to open source file {}", g_compilerOptions.From);
        return CompilerRetCodes::FailedToOpenSource;
    }

    CompilerRetCodes returnCode = CompilerRetCodes::Ok;

    if (!g_compilerOptions.DecompileTo.empty())
    {
        // Decompiler mode
        inFile.seekg(0, std::ifstream::end);
        size_t bufferSize = inFile.tellg();
        inFile.seekg(0, std::ifstream::beg);

        if (bufferSize == 0)
        {
            spdlog::error("--from file is empty!");
            inFile.close();
            return CompilerRetCodes::BadSourceFile;
        }

        auto buffer = std::make_unique<char[]>(bufferSize);
        inFile.read(buffer.get(), bufferSize);

        if (!inFile)
        {
            spdlog::error("Failed to read --from file contents! Required bytes {} but read only {} bytes", bufferSize, inFile.gcount());
            inFile.close();
            return CompilerRetCodes::BadSourceFile;
        }

        inFile.close();

        // Run decompiler
        auto root = BM::LOC::LOCTreeNode::ReadFromMemory(buffer.get(), bufferSize);
        if (!root)
        {
            spdlog::error("Failed to decompile LOC file {}", g_compilerOptions.From);
            return CompilerRetCodes::BadSourceFile;
        }

        spdlog::info("Decompiled!");
    }
    else
    {
        nlohmann::json localizationInJson;
        inFile >> localizationInJson;
        inFile.close();

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

        std::fstream outFile { g_compilerOptions.To, std::ios::app };
        if (!outFile)
        {
            spdlog::error("LOCC::Error: Failed to destination file {}", g_compilerOptions.To);
            return CompilerRetCodes::FailedToOpenDestination;
        }

        bool isCompiled = false;
        bool isWritten = false;

        std::vector<uint8_t> compiledBuffer;

        try {
            if (!BM::LOC::LOCTreeNode::Compile(root, compiledBuffer))
            {
                isCompiled = false;
            }
            else
            {
                isCompiled = true;
                outFile.write((char*)compiledBuffer.data(), compiledBuffer.size());
                if (!outFile)
                {
                    spdlog::error("LOCC::Error: Failed to write compiled bytecode. Requested {} bytes but read {} bytes", outFile.gcount(), compiledBuffer.size());
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

        outFile.close();

        if (isCompiled && isWritten)
        {
            if (g_compilerOptions.TestFinalFile)
            {
                //spdlog::info(" Run tests ...");
                // Not implemented yet
            }
        }
    }

    return static_cast<int>(returnCode);
}