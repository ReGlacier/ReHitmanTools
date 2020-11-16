#include <CompilerOptionsStorage.h>
#include <Decompiler.h>
#include <FIO.h>

#include <BM/LOC/LOCTreeCompiler.h>
#include <BM/LOC/LOCTreeFactory.h>
#include <BM/LOC/LOCTree.h>
#include <BM/LOC/LOCJson.h>

using namespace BM::LOC;

static constexpr int kPrettifyJsonIndentValue = 4;
static constexpr int kNoPrettifyJsonIndentValue = -1;

LOCC::ToolExitCodes LOCC::Decompile(std::string_view from, std::string_view to)
{
    spdlog::info("LOCC::Decompile| Decompiling {} to {} ...", from, to);

    size_t sourceBufferSize = 0;
    auto sourceBuffer = FIO::ReadFile(from, sourceBufferSize);
    if (!sourceBuffer)
    {
        spdlog::error("LOCC::Decompile| Failed to read source file {}", from);
        return LOCC::ToolExitCodes::BadSourceFile;
    }

    auto root = LOCTreeNode::ReadFromMemory(sourceBuffer.get(), sourceBufferSize, CompilerOptions.SupportMode);
    if (!root)
    {
        spdlog::error("LOCC::Decompile| Failed to decompile tree. Probably, you forgot to specify the game?");
        return LOCC::ToolExitCodes::BadSourceFormat;
    }

    spdlog::info("LOCC::Decompile| Decompiled! Serializing to JSON ...");

    nlohmann::json outJson;

    try
    {
        nlohmann::adl_serializer<LOCTreeNode>::to_json(outJson, root);
    }
    catch (const nlohmann::json::exception& jsonSerErr)
    {
        delete root;
        spdlog::error("LOCC::Decompile| Failed to serialize tree into JSON format. Reason: {}", jsonSerErr.what());
        return LOCC::ToolExitCodes::FailedToSerialize;
    }

    delete root;

    spdlog::info("LOCC::Decompile| Serialized! Saving to disk ...");

    if (!FIO::WriteFile(to, outJson, CompilerOptions.PrettifyOutputJson ? kPrettifyJsonIndentValue : kNoPrettifyJsonIndentValue))
    {
        spdlog::error("LOCC::Decompile| Failed to save serialized json to file {}!", to);
        return LOCC::ToolExitCodes::FailedToSaveSerializedResult;
    }

    spdlog::info("LOCC::Decompile| Source LOC {} decompiled to {} JSON successfully!", from, to);

    return LOCC::ToolExitCodes::Success;
}