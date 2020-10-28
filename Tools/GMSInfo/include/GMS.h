#pragma once

#include <string>
#include <string_view>
#include <vector>
#include <memory>

#include <GlacierTypeDefs.h>

namespace ReGlacier
{
    class LevelContainer;
    struct LevelAssets;

    struct GMSLinkRef
    {
        uint32_t index = 0u;

        struct {
            uint32_t index = 0;
            Glacier::TypeId id = Glacier::NOT_INITIALISED;
        } typeInfo;

        GMSLinkRef() = default;
        GMSLinkRef(uint32_t _index, uint32_t _typeId, bool isDeclared)
            : index(_index)
        {
            typeInfo.index = _typeId;
            typeInfo.id = isDeclared ? static_cast<Glacier::TypeId>(_typeId) : Glacier::NOT_FOUND;
        }
    };

    class GMS
    {
        bool m_isLoaded { false };
        std::string m_name;
        LevelContainer* m_container;
        LevelAssets* m_assets;

        std::vector<std::string> m_excludedAnimationsList;

        int32_t m_totalLinkRefsCount = 0;
        std::vector<GMSLinkRef>  m_linkRefs;
    public:
        using Ptr = std::unique_ptr<GMS>;

        GMS(std::string  name, LevelContainer* levelContainer, LevelAssets* levelAssets);

        void Load();
        bool SaveUncompressed(const std::string& filePath);
        void PrintInfo();

        [[nodiscard]] const std::vector<std::string>& GetExcludedAnimations() const;
        [[nodiscard]] const std::vector<GMSLinkRef>& GetLinkReferences() const;
    private:
        void LoadEntities(std::unique_ptr<char[]>&& buffer);
        void LoadImportTable(const char* gmsBuffer);
        void LoadExcludedAnimations(char* gmsBuffer, char* bufBuffer);

        std::unique_ptr<char[]> GetRawGMS(int& bufferSize);
    };
}