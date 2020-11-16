#include <CompilerOptionsStorage.h>

namespace LOCC
{
    const std::map<std::string, UtilityMode> CompilerOptionsStorage::Consts::ModesMap = {
            {"compile", UtilityMode::Compiler},
            {"decompile", UtilityMode::Decompiler}
    };

    const std::map<std::string, BM::LOC::LOCSupportMode> CompilerOptionsStorage::Consts::SupportModesMap = {
            {"bloodmoney", BM::LOC::LOCSupportMode::Hitman_BloodMoney},
            {"contracts", BM::LOC::LOCSupportMode::Hitman_Contracts},
            {"2sa", BM::LOC::LOCSupportMode::Hitman_2SA},
            {"a47", BM::LOC::LOCSupportMode::Hitman_A47}
    };

    CompilerOptionsStorage CompilerOptions {};
}