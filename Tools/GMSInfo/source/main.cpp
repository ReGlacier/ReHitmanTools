/**
 GMS Tool for Hitman Blood Money
 **/
#include <string>

#include <spdlog/spdlog.h>

#include <TypesDataBase.h>
#include <LevelDescription.h>

static constexpr const char* kDefaultTypeStorageFile = "typeids.json";

int main(int argc, char** argv)
{
    /*argh::parser cli(argc, argv);

    bool printLevelInfo;
    bool ignoreGMS { false };
    bool ignoreANM { false };
    bool ignoreLOC { false };
    bool ignorePRM { false };
    bool ignorePRP { false };
    bool ignoreTEX { false };
    bool ignoreSND { false };

    std::string levelArchivePath;
    std::string typesDataBaseFilePath;
    std::string uncompressedGMSPath;
    std::string exportLocalizationToFilePath;

    if (!(cli({"-L", "\"--level\""}) >> levelArchivePath))
    {
        spdlog::error("You must present path to the level archive!");
        return -1;
    }

    cli("--types", kDefaultTypeStorageFile) >> typesDataBaseFilePath;
    cli("--export-gms", "") >> uncompressedGMSPath;
    cli("--print-info", false) >> printLevelInfo;
    cli("--export-loc", "") >> exportLocalizationToFilePath;

    // 'Ignore' flags
    cli("--ignore-gms", false) >> ignoreGMS;
    cli("--ignore-anm", false) >> ignoreANM;
    cli("--ignore-loc", false) >> ignoreLOC;
    cli("--ignore-prm", false) >> ignorePRM;
    cli("--ignore-prp", false) >> ignorePRP;
    cli("--ignore-tex", false) >> ignoreTEX;
    cli("--ignore-snd", false) >> ignoreSND;

    if (!ReGlacier::TypesDataBase::GetInstance().Load(typesDataBaseFilePath))
    {
        spdlog::error("Failed to load types database from file {}", typesDataBaseFilePath);
        return -2;
    }

    // Open level archive
    auto level = std::make_unique<ReGlacier::LevelDescription>(levelArchivePath);
    if (!level->Open())
    {
        spdlog::error("Failed to open level {}", levelArchivePath);
        return -1;
    }

    {
        // Setup ignore flags
        level->SetIgnoreGMSFlag(ignoreGMS);
        level->SetIgnoreANMFlag(ignoreANM);
        level->SetIgnoreLOCFlag(ignoreLOC);
        level->SetIgnorePRMFlag(ignorePRM);
        level->SetIgnorePRPFlag(ignorePRP);
        level->SetIgnoreTEXFlag(ignoreTEX);
        level->SetIgnoreSNDFlag(ignoreSND);
    }


    level->LoadAndAnalyze();

    if (printLevelInfo)
        level->PrintInfo();

    if (!uncompressedGMSPath.empty())
        level->ExportUncompressedGMS(uncompressedGMSPath);

    if (!exportLocalizationToFilePath.empty())
    {
        if (ignoreLOC)
        {
            spdlog::warn("--export-loc option was ignored because LOC file was excluded from analysis by user");
        }
        else
        {
            if (level->ExportLocalizationToJson(exportLocalizationToFilePath))
            {
                spdlog::info("Localization exported to file {}", exportLocalizationToFilePath);
            } else {
                spdlog::error("Failed to export localization contents. More details in log.");
            }
        }
    }
    */
    return 0;
}