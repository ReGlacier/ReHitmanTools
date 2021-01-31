#include <GMS/GMS.h>
#include <GMS/GMSTypes.h>
#include <GMS/ADL/GMSADL.h>

#include <LevelContainer.h>
#include <GlacierTypeDefs.h>
#include <TypesDataBase.h>
#include <LevelAssets.h>

#include <BinaryWalker.h>
#include <BinaryWalkerADL.h>

#include <spdlog/spdlog.h>

#include <utility>
#include <fstream>
#include <numeric>
#include <set>

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
    bool GMS_Decompress(GMS2* gms, uint8_t* gmsDecompressed, int gmsInLength) {
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
    enum GMSOffsets : unsigned int {
        ExcludedAnimationsRegionAddr = 0x11,
        WeaponHandlesRegionAddr = 0x10
    };

    GMS::GMS(std::string name, LevelContainer* levelContainer, LevelAssets* levelAssets)
        : IGameEntity(name, levelContainer, levelAssets) {}

    bool GMS::Load()
    {
        assert(!m_isLoaded);

        size_t gmsBufferSize = 0;
        auto gmsBuffer = GetRawGMS(gmsBufferSize);

        if (!gmsBuffer)
        {
            spdlog::error("GMS::Load| Failed to load GMS {}", m_name);
            return false;
        }

        size_t prmBufferSize = 0;
        auto prmBuffer = m_container->Read(m_assets->PRM, prmBufferSize);
        if (!prmBuffer)
        {
            spdlog::error("GMS::Load| Failed to load PRM {}", m_assets->PRM);
            return false;
        }

        size_t bufBufferSize = 0;
        auto bufBuffer = m_container->Read(m_assets->BUF, bufBufferSize);
        if (!bufBuffer)
        {
            spdlog::error("GMS::Load| Failed to load BUF {}", m_assets->BUF);
            return false;
        }

        BinaryWalker gmsBinaryWalker(gmsBuffer.get(), gmsBufferSize);
        BinaryWalker prmBinaryWalker(prmBuffer.get(), prmBufferSize);
        BinaryWalker bufBinaryWalker(bufBuffer.get(), bufBufferSize);

        SGMSUncompressedHeader header {};
        BinaryWalkerADL<SGMSUncompressedHeader>::Read(gmsBinaryWalker, header);

        spdlog::info("TotalEntitiesCountPos: {:X}", header.TotalEntitiesCountPos);

        gmsBinaryWalker.Seek(header.TotalEntitiesCountPos, BinaryWalker::SeekType::FROM_BEGIN);
        m_totalEntities = gmsBinaryWalker.Read<int32_t>();

        spdlog::info("At TECP: +{:X}", gmsBinaryWalker.GetPosition());

        m_geoms.reserve(m_totalEntities);

        for (int entryId = 0; entryId < m_totalEntities; ++entryId)
        {
            gmsBinaryWalker.Seek(header.TotalEntitiesCountPos + (8 * entryId), BinaryWalker::SeekType::FROM_BEGIN);
            SGMSEntry entry {};
            BinaryWalkerADL<SGMSEntry>::Read(gmsBinaryWalker, entry);

            gmsBinaryWalker.Seek(4 * (entry.TypeInfoPos & 0xFFFFFF), BinaryWalker::SeekType::FROM_BEGIN); //& 0xFFFFFF IT'S VERY IMPORTANT!!!

            auto& info = m_geoms.emplace_back();
            info.id = entryId;

            BinaryWalkerADL<SGMSBaseGeom>::Read(gmsBinaryWalker, info.baseGeom);

            bufBinaryWalker.Seek(info.baseGeom.PrimitiveBufGroupNameOffset, BinaryWalker::BEGIN);
            BinaryWalkerADL<std::string>::Read(bufBinaryWalker, info.groupName);
        }

        m_isLoaded = true;
        return true;
    }

    bool GMS::SaveUncompressed(const std::string& filePath)
    {
        std::fstream stream(filePath, std::ios::out | std::ios::binary | std::ios::app);
        if (!stream)
        {
            spdlog::error("GMS::SaveUncompressed| Failed to create file {} to write binary", filePath);
            return false;
        }

        size_t uncompressedBufferSize = 0;
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
        {
            if (!m_excludedAnimationsList.empty())
            {
                spdlog::info("GMS| Excluded animations");
                for (const auto& anim : m_excludedAnimationsList)
                {
                    spdlog::info(" * {}", anim);
                }
            }
            else
            {
                spdlog::info("GMS| No excluded animations");
            }
        }

        {
            if (!m_weaponHandles.empty())
            {
                spdlog::info("Weapon handles (total {}):", m_weaponHandlesCount);
                spdlog::info("------------------------------");

                spdlog::info("#     | ID     | Unknown1 | Unknown 2");
                for (int i = 0; i < m_weaponHandles.size(); i++)
                {
                    spdlog::info("{:04d}    {:04X}     {:08X}   {:08X}", i, m_weaponHandles[i].entityId, m_weaponHandles[i].m_field4, m_weaponHandles[i].m_field8);
                }
            }
            else
            {
                spdlog::info("GMS::LoadWeaponHandles| No weapon handles declared there. Probably, you work with LoaderSequence.GMS");
            }
        }

        {
            spdlog::info("GMS Geoms: ");
            spdlog::info("    ID   |            Entity Name            |        Type Name        |    Type ID    ");
            for (const auto& geom : m_geoms)
            {
                spdlog::info("{:08X} {:33} {:23} {:8X}", geom.id, geom.groupName, Glacier::GetTypeIdAsString(geom.baseGeom.TypeId), geom.baseGeom.TypeId);
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

    const std::vector<GMSComposedInfoHolder> & GMS::GetGeoms() const {
        return m_geoms;
    }

    std::unique_ptr<uint8_t[]> GMS::GetUncompressedBuffer(unsigned int& uncompressedSize)
    {
        return GetRawGMS(uncompressedSize);
    }

    bool GMS::LoadEntities(std::unique_ptr<char[]>&& gmsBuffer, size_t bufferSize)
    {
        std::unique_ptr<char[]> data = std::move(gmsBuffer);
        char* buffer = data.get();

        size_t prmSize = 0, bufSize = 0;

        auto prm = m_container->Read(m_assets->PRM, prmSize);
        auto buf = m_container->Read(m_assets->BUF, bufSize);

        if (!prm)
        {
            spdlog::error("GMS::LoadEntities() Failed to read PRM file {}", m_assets->PRM);
            return false;
        }

        if (!buf)
        {
            spdlog::error("GMS::LoadEntities() Failed to read BUF file {}", m_assets->BUF);
            return false;
        }

        auto BUFBuffer = reinterpret_cast<char*>(buf.get());

        const bool importTablesOk = LoadImportTable(buffer, bufferSize);
        const bool propertiesOk = LoadProperties(buffer, bufferSize);
        const bool excludedAnimsOk = LoadExcludedAnimations(buffer, bufferSize, BUFBuffer, bufSize);
        const bool weaponsHandlesOk = LoadWeaponHandles(buffer, bufferSize, BUFBuffer, bufSize);

        spdlog::info("GMS::LoadEntities() = {} {} {} {}", importTablesOk, propertiesOk, excludedAnimsOk, weaponsHandlesOk);

        return importTablesOk || propertiesOk || excludedAnimsOk || weaponsHandlesOk;
    }

    bool GMS::LoadImportTable(const char* buffer, size_t bufferSize)
    {
        if (*(int*)buffer > bufferSize) {
            return false;
        }

        m_totalLinkRefsCount = *(int*)&buffer[*(int*)buffer];

        int entityLocator = 0;
        uint32_t entityIndex = 0;

        auto& db = TypesDataBase::GetInstance();

        m_linkRefs.reserve(m_totalLinkRefsCount);

        //while (true)
        do
        {
            //TODO: Think how to wrap that into the structure
            entityLocator = *(int*)(&buffer[8 * entityIndex + 4] + *(int*)buffer) & 0xFFFFFF;
            unsigned int entityType = *(int*)(&buffer[4 * entityLocator + 20]);
            //const bool isLoaderContents = entityType == Glacier::TypeId::ZLoader_Sequence_Setup_ZSTDOBJ;

            //int ptr = (int)&buffer[4 * entityLocator];
            m_linkRefs.emplace_back(entityIndex, entityType, db.HasDefinitionForEntityTypeId(entityType));

            ++entityIndex;
        } while (entityIndex != m_totalLinkRefsCount);

        //result = &prmBuffer[*((int*)v7 + 3)];

        return true;
    }

    bool GMS::LoadProperties(const char* buffer, size_t bufferSize)
    {
        return true;
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

    bool GMS::LoadExcludedAnimations(char* gmsBuffer, size_t gmsBufferSize, char* bufBuffer, size_t bufBufferSize)
    {
        auto excludedAnimationsOffset = ((int*)gmsBuffer)[GMSOffsets::ExcludedAnimationsRegionAddr];
        if (excludedAnimationsOffset >= bufBufferSize)
        {
            return false;
        }

        auto totalExcludedAnimations  = ((int*)bufBuffer)[excludedAnimationsOffset / sizeof(int)];
        if (excludedAnimationsOffset + 4 >= bufBufferSize)
        {
            return false;
        }

        auto excludedAnimationsBuffer = (char*)&((char*)bufBuffer)[excludedAnimationsOffset + 4];
        auto parsedAnimationsList = ParseIOISmartString(excludedAnimationsBuffer, totalExcludedAnimations);

        parsedAnimationsList.reserve(parsedAnimationsList.size());

        for (auto&& animName : parsedAnimationsList)
        {
            m_excludedAnimationsList.emplace_back(animName.data(), animName.length());
        }

        return true;
    }

    bool GMS::LoadWeaponHandles(char* gmsBuffer, size_t gmsBufferSize, char* bufBuffer, size_t bufBufferSize)
    {
        auto weaponHandlesOffset = ((int*)gmsBuffer)[GMSOffsets::WeaponHandlesRegionAddr];
        if (!weaponHandlesOffset)
        {
            // Note: I don't know how to check it more correctly but if weaponHandlesOffset == 0 we will have bad pointer
            return false;
        }

        if (weaponHandlesOffset / sizeof(int) >= bufBufferSize)
        {
            return false;
        }

        auto weaponHandlesCountPtr = (int*)&((int*)bufBuffer)[weaponHandlesOffset / sizeof(int)];

        m_weaponHandlesCount = *weaponHandlesCountPtr;

        auto weaponHandlesLocation = reinterpret_cast<char*>(weaponHandlesCountPtr + 1);

        if (!m_weaponHandlesCount)
        {
            return true;
        }

        m_weaponHandles.reserve(m_weaponHandlesCount);

        auto handles = std::make_unique<GMSWeaponHandle[]>(m_weaponHandlesCount);
        std::memcpy(handles.get(), weaponHandlesLocation, sizeof(GMSWeaponHandle) * m_weaponHandlesCount);

        for (int i = 0; i < m_weaponHandlesCount; i++)
        {
            m_weaponHandles.emplace_back(handles[i].entityId, handles[i].m_field4, handles[i].m_field8);
        }

        return true;
    }

    std::unique_ptr<uint8_t[]> GMS::GetRawGMS(size_t& outBufferSize)
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

        outBufferSize = (v5 + 15) & 0xFFFFFFF0;

        auto outBuffer = std::make_unique<uint8_t[]>(outBufferSize);
        Legacy::GMS_Decompress(&gms, outBuffer.get(), outBufferSize);

        return std::move(outBuffer);
    }

    /**
     * SND buffer usage note
     *
     * @ref0 unsigned int __thiscall sub_5A4620(int this, const void *a2, unsigned int a3)
     * @ref1 ZDllSound::GetStreamFilename
     * @ref2 _DWORD *__thiscall sub_4C82A0(void *this, int a2, int a3)
     * @ref3 int __thiscall sub_4C80C0(int this, int a2)
     * @ref4 int __thiscall sub_4C80F0(int this, int a2, int a3)
     *
     * OCT buffer usage notes
     *
     * @ref0 int __cdecl sub_40DEC0(int a1, int a2, int a3)
     * @ref1 int __thiscall ZEngineDataBase::CreateBoundTrees(ZEngineDataBase *this)
     */
}