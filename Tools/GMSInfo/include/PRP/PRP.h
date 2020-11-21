#pragma once

#include <IGameEntity.h>
#include <PRP/PRPTreeNode.h>

#include <string>
#include <vector>

namespace ReGlacier
{
    struct DictNode;

    class PRP : public IGameEntity
    {
    public:
        using Ptr = std::unique_ptr<PRP>;

        PRP(std::string  name, LevelContainer* levelContainer, LevelAssets* levelAssets);
        ~PRP();

        bool Load() override;
        void PrintInfo() override;

    private:
        void TryToDecompileEntities(std::unique_ptr<uint8_t[]>&& buffer, size_t bufferSize);

    private:
        int m_keysCount { 0 };
        int m_valuesOffset { 0 };
        std::vector<std::string> m_keys {};
        PRPTreeNode* m_tree { nullptr };
    };
}