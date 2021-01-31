#pragma once

#include <IGameEntity.h>

#include <PRP/PRPTypes.h>

#include <BinaryWalker.h>
#include <BinaryWalkerADL.h>

#include <string>
#include <vector>

namespace ReGlacier
{
    class PRP : public IGameEntity
    {
    public:
        using Ptr = std::unique_ptr<PRP>;

        PRP(std::string  name, LevelContainer* levelContainer, LevelAssets* levelAssets);
        ~PRP();

        bool Load() override;
        void PrintInfo() override;
    };
}