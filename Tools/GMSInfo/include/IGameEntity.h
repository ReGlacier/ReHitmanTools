#pragma once

#include <cstdlib>
#include <memory>
#include <string>
#include <string_view>

namespace ReGlacier
{
    class LevelContainer;
    struct LevelAssets;

    class IGameEntity
    {
    protected:
        bool m_isLoaded { false };
        LevelContainer* m_container;
        LevelAssets* m_assets;
        std::string m_name;

    public:
        virtual ~IGameEntity() noexcept = default;

        IGameEntity(const std::string& name, LevelContainer* levelContainer, LevelAssets* levelAssets)
            : m_container(levelContainer)
            , m_assets(levelAssets)
            , m_name(name)
        {
        }

        virtual bool Load() = 0;
    };
}