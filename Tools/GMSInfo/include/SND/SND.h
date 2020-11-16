#pragma once

#include <IGameEntity.h>

namespace ReGlacier
{
    class SND : public IGameEntity
    {
    public:
        using Ptr = std::unique_ptr<SND>;

        SND(const std::string& name, LevelContainer* levelContainer, LevelAssets* levelAssets);

        bool Load() override;
    };
}