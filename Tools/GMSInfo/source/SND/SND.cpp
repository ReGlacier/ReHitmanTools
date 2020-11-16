#include <SND/SND.h>
#include <SND/SNDTypes.h>
#include <SND/ADL/SNDADL.h>

#include <GlacierTypeDefs.h>
#include <LevelContainer.h>
#include <TypesDataBase.h>
#include <LevelAssets.h>

#include <spdlog/spdlog.h>

#include <BinaryWalker.h>
#include <BinaryWalkerADL.h>

#include <utility>
#include <array>

namespace ReGlacier
{
    SND::SND(const std::string& name, LevelContainer* levelContainer, LevelAssets* levelAssets)
        : IGameEntity(name, levelContainer, levelAssets)
    {
    }

    bool SND::Load()
    {
        return true;
    }
}