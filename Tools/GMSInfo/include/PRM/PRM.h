#pragma once

#include <IGameEntity.h>

namespace ReGlacier
{
    class PRM : public IGameEntity
    {
    public:
        using Ptr = std::unique_ptr<PRM>;

        PRM(std::string  name, LevelContainer* levelContainer, LevelAssets* levelAssets);

        bool Load() override;
    };
}