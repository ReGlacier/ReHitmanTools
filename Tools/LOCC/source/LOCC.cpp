/**
 * LOC File Compiler
 */
// --- LOC Compiler headers
#include <LOCC.h>
#include <FIO.h>
#include <ResourceCollection.h>
#include <Compiler.h>
#include <Decompiler.h>
#include <ToolExitCodes.h>
#include <CompilerOptionsStorage.h>

// --- CLI11 lib
#include <CLI/App.hpp>
#include <CLI/Formatter.hpp>
#include <CLI/Config.hpp>

// --- BloodMoney LOC Format support engine
#include <BM/LOC/LOCTypes.h>
#include <BM/LOC/LOCJson.h>
#include <BM/LOC/LOCTree.h>

int main(int argc, char** argv)
{
    CLI::App app { "LOC Compiler Utility" };

    app.add_option("--f,--from", LOCC::CompilerOptions.From, "Path to file who will be compiled/decompiled")->required();
    app.add_option("--t,--to", LOCC::CompilerOptions.To, "Path to file who will be the result of compilation/decompilation")->required();
    app.add_option("--p,--pretty,--pretty-json", LOCC::CompilerOptions.PrettifyOutputJson, "Make result JSON more human-readable");
    app.add_option("--m,--mode", LOCC::CompilerOptions.ToolMode, "Tool mode")
        ->transform(CLI::CheckedTransformer(LOCC::CompilerOptionsStorage::Consts::ModesMap, CLI::ignore_case));
    app.add_option("--g,--game", LOCC::CompilerOptions.SupportMode, "Support mode")
        ->transform(CLI::CheckedTransformer(LOCC::CompilerOptionsStorage::Consts::SupportModesMap, CLI::ignore_case));

    CLI11_PARSE(app, argc, argv);

    if (!LOCC::FIO::HasFile(LOCC::CompilerOptions.From))
    {
        spdlog::error("LOCC| Source file {} not found!", LOCC::CompilerOptions.From);
        return -1;
    }

    LOCC::ToolExitCodes exitCode = LOCC::ToolExitCodes::Success;

    switch (LOCC::CompilerOptions.ToolMode)
    {
        case LOCC::UtilityMode::Compiler:
            exitCode = LOCC::Compile(LOCC::CompilerOptions.From, LOCC::CompilerOptions.To);
            break;
        case LOCC::UtilityMode::Decompiler:
            exitCode = LOCC::Decompile(LOCC::CompilerOptions.From, LOCC::CompilerOptions.To);
            break;
        default:
            spdlog::warn("LOCC| No command...");
            break;
    }

    return exitCode;
}