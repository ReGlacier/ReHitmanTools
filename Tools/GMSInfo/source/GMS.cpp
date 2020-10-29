#include <GMS.h>
#include <LevelAssets.h>
#include <LevelContainer.h>
#include <ZPackedDataChunk.h>
#include <GlacierTypeDefs.h>
#include <TypesDataBase.h>

#include <spdlog/spdlog.h>

#include <utility>
#include <fstream>

extern "C" {
#include <zlib.h>
}

namespace Legacy
{
    struct GMS2
    {
        int field_0;
        int m_raw;
        int field_8;
        int field_C;
        int field_10;
        int field_14;
    };

    struct GMSFileEntry
    {
        int field_0;
        int field_4;
        int field_8;
    };

    [[deprecated("This function is an old implementation of IOI decoder. Please, rewrite it to nice C++ api")]]
    bool GMS_Decompress(GMS2* gms, char* gmsDecompressed, int gmsInLength) {
        if (gms->field_14 == 1) {
            const int length = (gms->field_C >= gmsInLength) ? gms->field_C : gmsInLength;
            memcpy(gmsDecompressed, (void*) (gms->m_raw + 9), length);
            return true;
        }

        auto entry = reinterpret_cast<GMSFileEntry*>(gms->m_raw);

        z_stream stream;
        stream.avail_in = gms->field_8;
        stream.next_in = (unsigned char*) &entry->field_8 + 1;
        stream.next_out = (Bytef*) gmsDecompressed;
        stream.avail_out = gmsInLength;
        stream.zalloc = Z_NULL;
        stream.zfree = Z_NULL;

        if (inflateInit2(&stream, -15) != Z_OK) {
            spdlog::error("inflateInit2() failed!");
            return false;
        }

        int result = inflate(&stream, Z_FINISH);
        inflateEnd(&stream);

        if ((result != Z_STREAM_END || result != Z_BUF_ERROR) && !stream.avail_out) {
            return false;
        }

        return true;
    }
}

namespace ReGlacier
{
    GMS::GMS(std::string name, LevelContainer* levelContainer, LevelAssets* levelAssets)
        : m_name(std::move(name))
        , m_container(levelContainer)
        , m_assets(levelAssets)
    {}

    void GMS::Load()
    {
        assert(!m_isLoaded);

        int bufferSize = 0;
        auto buffer = GetRawGMS(bufferSize);

        LoadEntities(std::move(buffer));

        m_isLoaded = true;
    }

    bool GMS::SaveUncompressed(const std::string& filePath)
    {
        std::fstream stream(filePath, std::ios::out | std::ios::binary | std::ios::app);
        if (!stream)
        {
            spdlog::error("GMS::SaveUncompressed| Failed to create file {} to write binary", filePath);
            return false;
        }

        int uncompressedBufferSize = 0;
        auto buff = GetRawGMS(uncompressedBufferSize);
        if (buff)
        {
            stream.write(reinterpret_cast<const char*>(buff.get()), uncompressedBufferSize);
            stream.flush();
        }
        else
        {
            spdlog::error("GMS::GetRawGMS() failed to extract raw contents");
            return false;
        }
        stream.close();
        
        return true;
    }

    void GMS::PrintInfo() {
        if (!m_isLoaded)
        {
            Load();
        }
        assert(m_isLoaded);

        {
            auto& db = TypesDataBase::GetInstance();

            spdlog::info("GMS Linking table");
            for (const auto& linkRef : m_linkRefs)
            {
                spdlog::info("#{:06d} as {}", linkRef.index, db.GetEntityTypeById(linkRef.typeInfo.index));
            }
        }

        {
            spdlog::info("GMS| Excluded animations");
            for (const auto& anim : m_excludedAnimationsList)
            {
                spdlog::info(" * {}", anim);
            }
        }
    }

    const std::vector<std::string> & GMS::GetExcludedAnimations() const
    {
        return m_excludedAnimationsList;
    }

    const std::vector<GMSLinkRef>& GMS::GetLinkReferences() const
    {
        return m_linkRefs;
    }

    void GMS::LoadEntities(std::unique_ptr<char[]>&& gmsBuffer)
    {
        std::unique_ptr<char[]> data = std::move(gmsBuffer);
        char* buffer = data.get();

        size_t prmSize = 0, bufSize = 0;

        auto prm = m_container->Read(m_assets->PRM, prmSize);
        auto buf = m_container->Read(m_assets->BUF, bufSize);

        if (!prm)
        {
            spdlog::error("GMS::LoadEntities() Failed to read PRM file {}", m_assets->PRM);
            return;
        }

        if (!buf)
        {
            spdlog::error("GMS::LoadEntities() Failed to read BUF file {}", m_assets->BUF);
            return;
        }

        auto BUFBuffer = reinterpret_cast<char*>(buf.get());

        LoadImportTable(buffer);
        LoadExcludedAnimations(buffer, BUFBuffer);
    }

    void GMS::LoadImportTable(const char* buffer)
    {
        m_totalLinkRefsCount = *(int*)&buffer[*(int*)buffer];

        int entityLocator = 0;
        uint32_t entityIndex = 0;

        auto& db = TypesDataBase::GetInstance();

        m_linkRefs.reserve(m_totalLinkRefsCount);

        while (true)
        {
            //TODO: Think how to wrap that into the structure
            entityLocator = *(int*)(&buffer[8 * entityIndex + 4] + *(int*)buffer) & 0xFFFFFF;
            unsigned int entityType = *(int*)(&buffer[4 * entityLocator + 20]);
            //const bool isLoaderContents = entityType == Glacier::TypeId::ZLoader_Sequence_Setup_ZSTDOBJ;

            //int ptr = (int)&buffer[4 * entityLocator];
            m_linkRefs.emplace_back(entityIndex, entityType, db.HasDefinitionForEntityTypeId(entityType));

            if (++entityIndex == m_totalLinkRefsCount)
            {
                return;
            }
        }

        //result = &prmBuffer[*((int*)v7 + 3)];
    }

    std::vector<std::string_view> ParseIOISmartString(const char* string, int awaitEntitiesCount)
    {
        if (!string || !awaitEntitiesCount)
            return {};

        std::vector<std::string_view> result;
        // First string not declared
        auto caretPointer = string;
        do {
            int lengthOfString = static_cast<int>(caretPointer[0]); // NOLINT(cert-str34-c)
            ++caretPointer;

            result.emplace_back(caretPointer, lengthOfString);
            caretPointer += lengthOfString;
            --awaitEntitiesCount;
        } while (awaitEntitiesCount);

        return result;
    }

    void GMS::LoadExcludedAnimations(char* gmsBuffer, char* bufBuffer)
    {
        auto excludedAnimationsOffset = ((int*)gmsBuffer)[0x11];
        auto totalExcludedAnimations  = ((int*)bufBuffer)[excludedAnimationsOffset / sizeof(int)];
        auto excludedAnimationsBuffer = (char*)&((char*)bufBuffer)[excludedAnimationsOffset + 4];
        auto parsedAnimationsList = ParseIOISmartString(excludedAnimationsBuffer, totalExcludedAnimations);

        parsedAnimationsList.reserve(parsedAnimationsList.size());

        for (auto&& animName : parsedAnimationsList)
        {
            m_excludedAnimationsList.emplace_back(animName.data(), animName.length());
        }
    }

    std::unique_ptr<char[]> GMS::GetRawGMS(int& outBufferSize)
    {
        size_t bufferSize = 0;
        auto buffer = m_container->Read(m_name, bufferSize);

        if (!buffer) {
            spdlog::error("GMS::GetRawGMS() | Unable to read GMS file {}", m_name);
            return nullptr;
        }

        // Uncompress (legacy, TODO: Refactor!)
        auto raw = reinterpret_cast<char*>(buffer.get());

        Legacy::GMS2 gms = { 0 };
        gms.field_0 = 1;
        gms.m_raw = (int)raw;

        int v5 = *(int*)raw;

        gms.field_C = v5;
        gms.field_8 = *(unsigned int*)(raw + 4);
        gms.field_14 = (*(unsigned char*)(raw + 8)) != 0;

        outBufferSize = (v5 + 15) & 0xFFFFFFF0; ///GOT WRONG SIZE

        auto outBuffer = std::make_unique<char[]>(outBufferSize);
        Legacy::GMS_Decompress(&gms, outBuffer.get(), outBufferSize);

        return std::move(outBuffer);
    }
}