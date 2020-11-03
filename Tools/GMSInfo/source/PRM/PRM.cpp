#include <PRM/PRM.h>
#include <LevelAssets.h>
#include <LevelContainer.h>
#include <PRM/PRMTypes.h>
#include <GlacierTypeDefs.h>
#include <TypesDataBase.h>

#include <BinaryWalker.h>
#include <BinaryWalkerADL.h>

#include <spdlog/spdlog.h>

#include <utility>
#include <fstream>

namespace ReGlacier
{
    static constexpr size_t kMeshSize = 0x40;

    PRM::PRM(std::string name, LevelContainer* levelContainer, LevelAssets* levelAssets)
        : IGameEntity(name, levelContainer, levelAssets) {}

    bool PRM::Load()
    {
        //TODO: Rewrite to binwalk!
        return true;
    }
}