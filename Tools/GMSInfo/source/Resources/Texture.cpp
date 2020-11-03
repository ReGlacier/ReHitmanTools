#include <Resources/Texture.h>
#include <Resources/TextureImporter.h>

#include <spdlog/spdlog.h>

#include <fstream>

namespace ReGlacier
{
    int32_t Texture::GetWidth() const { return m_width; }
    int32_t Texture::GetHeight() const { return m_height; }
    int32_t Texture::GetMip() const { return m_mipLevel; }
    ETEXEntityType Texture::GetEntityType() const { return m_gameTexType; }
    std::string_view Texture::GetName() const { return m_name; }
    float Texture::GetUnknownFloat2() const { return m_unknown2; }
}