#pragma once

#include <cstdlib>

#include <memory>
#include <string>
#include <vector>
#include <optional>

#include <TEX/TEXTypes.h>

namespace ReGlacier
{
    enum class TextureFormat
    {
        DDS, PNG, JPG
    };

    class Texture
    {
        friend class TEX;
    public:
        using Ptr = std::shared_ptr<Texture>;
        using Ref = std::weak_ptr<Texture>;

        Texture() = default;

        [[nodiscard]] int32_t GetWidth() const;
        [[nodiscard]] int32_t GetHeight() const;
        [[nodiscard]] int32_t GetMip() const;
        [[nodiscard]] ETEXEntityType GetEntityType() const;
        [[nodiscard]] std::string_view GetName() const;
        [[nodiscard]] float GetUnknownFloat2() const;

    protected:
        int32_t m_width;
        int32_t m_height;
        int32_t m_mipLevel;
        int32_t m_indicesCount;
        float m_unknown2;
        ETEXEntityType m_gameTexType;
        std::string m_name;

        std::vector<STEXEntityAllocationInfo> m_allocationInfoPool;
        std::optional<SPALPaletteInfo> m_PALPaletteData;
        std::vector<int32_t> m_indices;
    };
}