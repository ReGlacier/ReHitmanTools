/**
 GMS Tool for Hitman Blood Money
 **/
#include <cstdint>
#include <cstring>
#include <cstdlib>
#include <cassert>
#include <cstdio>

#include <string>

#include <argh.h>
#include <spdlog/spdlog.h>

#include <TypesDataBase.h>
#include <GlacierTypeDefs.h>
#include <ZPackedDataChunk.h>
#include <LevelDescription.h>

int main(int argc, char** argv)
{
    argh::parser cli(argc, argv);
    std::string levelArchivePath;
    std::string typesDataBaseFilePath;

    if (!(cli({"-L", "\"--level\""}) >> levelArchivePath))
    {
        spdlog::error("You must present path to the level archive!");
        return -1;
    }

    cli("--types", "typeids.json") >> typesDataBaseFilePath;

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

    level->AnalyzeGMS();

    return 0;
}