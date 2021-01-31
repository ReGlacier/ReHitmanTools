#pragma once

#include <string>
#include <string_view>
#include <vector>
#include <memory>

#include <GMS/GMSTypes.h>
#include <IGameEntity.h>

namespace ReGlacier
{
    class LevelContainer;
    struct LevelAssets;

    class GMS : public IGameEntity
    {
        std::vector<std::string> m_excludedAnimationsList;

        int32_t m_totalLinkRefsCount = 0;
        std::vector<GMSLinkRef>  m_linkRefs;

        int32_t m_weaponHandlesCount = 0;
        std::vector<GMSWeaponHandle> m_weaponHandles;
    public:
        using Ptr = std::unique_ptr<GMS>;

        GMS(std::string  name, LevelContainer* levelContainer, LevelAssets* levelAssets);

        bool Load() override;
        bool SaveUncompressed(const std::string& filePath);
        void PrintInfo() override;

        [[nodiscard]] const std::vector<std::string>& GetExcludedAnimations() const;
        [[nodiscard,deprecated("Not working, please, use geoms instead of it")]] const std::vector<GMSLinkRef>& GetLinkReferences() const;
        [[nodiscard]] const std::vector<GMSComposedInfoHolder>& GetGeoms() const;

        [[nodiscard]] std::unique_ptr<uint8_t[]> GetUncompressedBuffer(unsigned int& uncompressedSize);
    private:
        bool LoadEntities(std::unique_ptr<char[]>&& buffer, size_t bufferSize);
        bool LoadImportTable(const char* gmsBuffer, size_t bufferSize);
        bool LoadProperties(const char* gmsBuffer, size_t bufferSize);
        bool LoadExcludedAnimations(char* gmsBuffer, size_t gmsBufferSize, char* bufBuffer, size_t bufBufferSize);
        bool LoadWeaponHandles(char* gmsBuffer, size_t gmsBufferSize, char* bufBuffer, size_t bufBufferSize);

        std::unique_ptr<uint8_t[]> GetRawGMS(size_t& bufferSize);

    private:
        int32_t m_totalEntities;

        std::vector<GMSComposedInfoHolder> m_geoms;
    };
}