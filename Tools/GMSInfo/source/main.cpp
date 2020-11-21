/**
 GMS Tool for Hitman Blood Money
 **/
#include <string>

#include <spdlog/spdlog.h>

#include <TypesDataBase.h>
#include <LevelDescription.h>

// CLI11
#include <CLI/App.hpp>
#include <CLI/Formatter.hpp>
#include <CLI/Config.hpp>

static constexpr const char* kDefaultTypeStorageFile = "typeids.json";

int main(int argc, char** argv)
{
    bool printLevelInfo { false };
    bool ignoreGMS { false };
    bool ignoreANM { false };
    bool ignoreLOC { false };
    bool ignorePRM { false };
    bool ignorePRP { false };
    bool ignoreTEX { false };
    bool ignoreSND { false };

    std::string levelArchivePath;
    std::string typesDataBaseFilePath = kDefaultTypeStorageFile;
    std::string uncompressedGMSPath;
    std::string exportLocalizationToFilePath;

    CLI::App app { "GMS Tool" };

    app.add_option("--level", levelArchivePath, "Path to level ZIP")->required();
    app.add_option("--types", typesDataBaseFilePath, "Set types DB JSON file");
    app.add_option("--export-gms", uncompressedGMSPath, "Export uncompressed GMS to specified file");
    app.add_option("--print-info", printLevelInfo, "Dump level info to console");
    app.add_option("--export-loc", exportLocalizationToFilePath, "Export decompiled LOC file into file at specified path");
    app.add_option("--ignore-gms", ignoreGMS, "Ignore .GMS file");
    app.add_option("--ignore-anm", ignoreANM, "Ignore .ANM file");
    app.add_option("--ignore-loc", ignoreLOC, "Ignore .LOC file");
    app.add_option("--ignore-prm", ignorePRM, "Ignore .PRM file");
    app.add_option("--ignore-prp", ignorePRP, "Ignore .PRP file");
    app.add_option("--ignore-tex", ignoreTEX, "Ignore .TEX file");
    app.add_option("--ignore-snd", ignoreSND, "Ignore .SND file");
    CLI11_PARSE(app, argc, argv);

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

    return 0;
}