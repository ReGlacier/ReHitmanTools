#pragma once

#include <IGameEntity.h>

namespace ReGlacier
{
    class ANM : public IGameEntity
    {
    public:
        using Ptr = std::unique_ptr<ANM>;

        ANM(const std::string& name, LevelContainer* levelContainer, LevelAssets* levelAssets);

        bool Load() override;
    };
}