#pragma once

#include <IGameEntity.h>

#include <Resources/Texture.h>

#include <vector>

namespace ReGlacier
{
    class TEX : public IGameEntity
    {
    public:
        using Ptr = std::unique_ptr<TEX>;

        TEX(std::string  name, LevelContainer* levelContainer, LevelAssets* levelAssets);

        bool Load() override;

        const std::vector<Texture::Ptr>& GetLoadedTextures() const;

    private:
        std::vector<Texture::Ptr> m_textures;
    };

}