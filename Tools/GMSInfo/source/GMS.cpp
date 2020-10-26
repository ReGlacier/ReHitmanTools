#include <GMS.h>
#include <LevelAssets.h>
#include <LevelContainer.h>
#include <ZPackedDataChunk.h>
#include <GlacierTypeDefs.h>
#include <TypesDataBase.h>

#include <spdlog/spdlog.h>

#include <utility>

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

    void GMS::PrintInfo() {
        size_t bufferSize = 0;
        auto buffer = m_container->Read(m_name, bufferSize);

        if (!buffer) {
            spdlog::error("GMS::PrintInfo() | Unable to read GMS file {}", m_name);
            return;
        }

        // Uncompress
        auto raw = reinterpret_cast<char*>(buffer.get());

        Legacy::GMS2 gms = { 0 };
        gms.field_0 = 1;
        gms.m_raw = (int)raw;

        int v5 = *(unsigned long*)raw;

        gms.field_C = v5;
        gms.field_8 = *(unsigned long*)(raw + 4);
        gms.field_14 = (*(unsigned char*)(raw + 8)) != 0;

        int outBuffSize = (v5 + 15) & 0xFFFFFFF0;

        auto outBuffer = std::make_unique<char[]>(outBuffSize);
        Legacy::GMS_Decompress(&gms, outBuffer.get(), outBuffSize);

        spdlog::info("??");
        EnumerateEntities(std::move(outBuffer));
    }

    void GMS::EnumerateEntities(std::unique_ptr<char[]>&& gmsBuffer)
    {
        std::unique_ptr<char[]> data = std::move(gmsBuffer);
        char* buffer = data.get();

        size_t prmSize = 0;
        size_t bufSize = 0;

        auto prm = m_container->Read(m_assets->PRM, prmSize);
        auto buf = m_container->Read(m_assets->BUF, bufSize);

        if (!prm)
        {
            spdlog::error("GMS::EnumerateEntities() Failed to read PRM file {}", m_assets->PRM);
            return;
        }

        if (!buf)
        {
            spdlog::error("GMS::EnumerateEntities() Failed to read BUF file {}", m_assets->BUF);
            return;
        }

#define G_AT(x) (*(((int*)buffer) + x))
        int flag_0x10 = G_AT(0x10);
        int flag_0x11 = G_AT(0x11);
#undef G_AT

        int totalEntitiesCount = *(int*)&buffer[*(int*)buffer];
        spdlog::info(" ---------------[ GMS ]--------------- ");
        spdlog::info("Total entities: {}", totalEntitiesCount);
        spdlog::info("Flag [0x10]: {:X}", flag_0x10);
        spdlog::info("Flag [0x11]: {:X}", flag_0x11);

        spdlog::info(" AF table");
        spdlog::info("--------------");
        int afEntitiesCount = *(int*)(&buf[flag_0x11]);
        spdlog::info("Total entities: {}", afEntitiesCount);

        int entityNum = 0;
        char* afPtr = (char*)(&buf[flag_0x11] + sizeof(int)) + 1;
        std::string_view afid { afPtr };

        // TODO: Loop
        spdlog::info("[{:4}] {}", entityNum, afid);

        spdlog::info(" Import table");
        spdlog::info("--------------");
        spdlog::info("Index |    Ref    |Pointer | Type ID");

        int entityLocator = 0;
        int entityIndex = 0;

        auto& db = TypesDataBase::GetInstance();

        while (true)
        {
            entityLocator = *(int*)(&buffer[8 * entityIndex + 4] + *(int*)buffer) & 0xFFFFFF;
            unsigned int entityType = *(int*)(&buffer[4 * entityLocator + 20]);
            const bool isLoaderContents = entityType == Glacier::TypeId::ZLoader_Sequence_Setup_ZSTDOBJ;

            int ptr = (int)&buffer[4 * entityLocator];
            std::string typeInfoStr = db.GetEntityTypeById(entityType);

            printf("#%.4d | %.9X | %.8X | %s\n", entityIndex, entityLocator, ptr, typeInfoStr.c_str());

            if (db.HasDefinitionForEntityTypeId(entityType) && static_cast<Glacier::TypeId>(entityType) == Glacier::TypeId::ZHitman3_ZPlayer)
            {
                printf("Player!\n");
            }

            if (isLoaderContents)
            {
                //TODO: Save XML
            }

            if (++entityIndex == totalEntitiesCount)
            {
                printf("--------------- END OF GMS TABLE ---------------\n");
                break;
            }
        }

        //result = &prmBuffer[*((int*)v7 + 3)];
    }
}