#pragma once

#include <IGameEntity.h>

namespace ReGlacier
{
    class TEX : public IGameEntity
    {
    public:
        using Ptr = std::unique_ptr<TEX>;

        TEX(std::string  name, LevelContainer* levelContainer, LevelAssets* levelAssets);

        bool Load() override;
    };

}