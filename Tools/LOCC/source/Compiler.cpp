#include <CompilerOptionsStorage.h>
#include <Compiler.h>
#include <FIO.h>

#include <BM/LOC/LOCTreeCompiler.h>
#include <BM/LOC/LOCTreeFactory.h>
#include <BM/LOC/LOCTree.h>
#include <BM/LOC/LOCJson.h>

using namespace BM::LOC;

LOCC::ToolExitCodes LOCC::Compile(std::string_view from, std::string_view to)
{
    spdlog::info("LOCC::Compile| Loading source JSON from {}", from);
    nlohmann::json sourceJson = FIO::ReadJson(from);
    if (sourceJson.empty())
    {
        spdlog::error("LOCC::Compile| Failed to load source JSON from file {}", from);
        return LOCC::ToolExitCodes::FailedToLoadJson;
    }

    spdlog::info("LOCC::Compile| Loaded! Trying to deserialize tree ...");
    auto root = LOCTreeFactory::Create();
    try
    {
        nlohmann::adl_serializer<LOCTreeNode>::from_json(sourceJson, root);
    }
    catch (const nlohmann::json::exception& badJsonEx)
    {
        spdlog::error("LOCC::Compile| Failed to deserialize source json {}. Error: {}", from, badJsonEx.what());
        delete root;
        return LOCC::ToolExitCodes::BadSourceFormat;
    }
    catch (const std::exception& badDataEx)
    {
        spdlog::error("LOCC::Compile| Failed to deserialize source json {}. STL Error: {}", from, badDataEx.what());
        delete root;
        return LOCC::ToolExitCodes::BadSourceFormat;
    }

    spdlog::info("LOCC::Compile| Done! Trying to compile final LOC ...");
    LOCTreeCompiler::Buffer compiledBuffer {};
    bool compileResult = false;
    try
    {
        compileResult = LOCTreeCompiler::Compile(compiledBuffer, root, CompilerOptions.SupportMode);
    }
    catch (const std::exception& compilerEx)
    {
        spdlog::error("LOCC::Compile| Compiler error! Message: {}", compilerEx.what());
        delete root;
        return LOCC::ToolExitCodes::CompileError;
    }

    delete root;

    if (!compileResult)
    {
        spdlog::error("LOCC::Compile| Compile failed. See log for details");
        return LOCC::ToolExitCodes::UnknownCompileError;
    }

    spdlog::info("LOCC::Compile| Compiled! Trying save to file {}", to);
    if (!FIO::WriteFile(to, (char*)compiledBuffer.data(), compiledBuffer.size()))
    {
        spdlog::error("LOCC::Compile| Failed to save output file to {}", to);
        return LOCC::ToolExitCodes::FailedToSaveCompiledResult;
    }

    spdlog::info("LOCC::Compile| Done! File {} compiled and saved to {}", from, to);
    return LOCC::ToolExitCodes::Success;
}