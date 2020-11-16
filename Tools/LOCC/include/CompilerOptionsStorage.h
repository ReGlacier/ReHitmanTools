#include <LOCC.h>

#include <BM/LOC/LOCSupportMode.h>

namespace LOCC
{
    enum class UtilityMode : int { Compiler, Decompiler };

    struct CompilerOptionsStorage
    {
        struct Consts
        {
            static const std::map<std::string, UtilityMode> ModesMap;
            static const std::map<std::string, BM::LOC::LOCSupportMode> SupportModesMap;
        };

        std::string             From;
        std::string             To;
        UtilityMode             ToolMode           { UtilityMode::Compiler };
        BM::LOC::LOCSupportMode SupportMode        { BM::LOC::LOCSupportMode::Generic };
        bool                    PrettifyOutputJson { false };
    };

    extern CompilerOptionsStorage CompilerOptions;
}