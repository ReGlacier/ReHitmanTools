#pragma once

#include <string>
#include <memory>

namespace ReGlacier
{
    class LevelContainer;
    struct LevelAssets;

    class GMS
    {
        std::string m_name;
        LevelContainer* m_container;
        LevelAssets* m_assets;
    public:
        GMS(std::string  name, LevelContainer* levelContainer, LevelAssets* levelAssets);

        void PrintInfo();

    private:
        void EnumerateEntities(std::unique_ptr<char[]>&& buffer);
    };
}