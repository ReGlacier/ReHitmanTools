/**
 GMS Tool for Hitman Blood Money
 **/
#include <string>

#include <argh.h>
#include <spdlog/spdlog.h>

#include <TypesDataBase.h>
#include <LevelDescription.h>

static constexpr const char* kDefaultTypeStorageFile = "typeids.json";

int main(int argc, char** argv)
{
    argh::parser cli(argc, argv);

    bool printLevelInfo;
    std::string levelArchivePath;
    std::string typesDataBaseFilePath;
    std::string uncompressedGMSPath;

    if (!(cli({"-L", "\"--level\""}) >> levelArchivePath))
    {
        spdlog::error("You must present path to the level archive!");
        return -1;
    }

    cli("--types", kDefaultTypeStorageFile) >> typesDataBaseFilePath;
    cli("--export-gms", "") >> uncompressedGMSPath;
    cli("--print-info", false) >> printLevelInfo;

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

    level->Analyze();

    if (printLevelInfo)
        level->PrintInfo();

    if (!uncompressedGMSPath.empty())
        level->ExportUncompressedGMS(uncompressedGMSPath);

    return 0;
}