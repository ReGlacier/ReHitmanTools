#include <PRM/PRM.h>
#include <PRM/PRMTypes.h>
#include <PRM/ADL/PRMADL.h>

#include <LevelAssets.h>
#include <LevelContainer.h>
#include <GlacierTypeDefs.h>
#include <TypesDataBase.h>

#include <BinaryWalker.h>
#include <BinaryWalkerADL.h>

#include <spdlog/spdlog.h>

#include <utility>
#include <fstream>

/**
 * @brief
 *      HWORD - Get high 16 bits of 32 bit number
 *      LWORD - Get low 16 bits of 32 bit number
 */
#define HWORD(v) (static_cast<uint16_t>(((static_cast<uint32_t>(v) >> 0x10) & 0xFFFF)))
#define LWORD(v) (static_cast<uint16_t>(static_cast<uint32_t>(v) & 0xFFFF))

namespace ReGlacier
{
    static constexpr size_t kMeshSize = 0x40;

    PRM::PRM(std::string name, LevelContainer* levelContainer, LevelAssets* levelAssets)
        : IGameEntity(name, levelContainer, levelAssets) {}

    bool PRM::Load()
    {
        return true; //Skip for speed

        size_t prmBufferSize = 0;
        auto prmBuffer = m_container->Read(m_name, prmBufferSize);

        if (!prmBuffer)
        {
            spdlog::error("PRM::Load| Failed to load file {}", m_name);
            return false;
        }

        BinaryWalker binaryWalker(prmBuffer.get(), prmBufferSize);

        PRMHeader header {};
        PRMChunk chunk {};

        // Read header
        BinaryWalkerADL<PRMHeader>::Read(binaryWalker, header);

        // Read root chunk
        binaryWalker.Seek(header.ChunkPos, BinaryWalker::SeekType::FROM_BEGIN);

        spdlog::info("Total chunks: {}, first chunk at +{:X}", header.ChunkNum, header.ChunkPos);
        spdlog::info("  #   |    Pos   |   Size   |  Is GEOM  |  Unknown ");
        int chunkId = 0;
        do {
            BinaryWalkerADL<PRMChunk>::Read(binaryWalker, chunk);
            {
                spdlog::info(
                        "#{:4d} | {:8X} | {:8X} | {:8X}  | {:8X}",
                        chunkId, chunk.Pos, chunk.Size, chunk.IsGeometry, chunk.Unknown2);

            }
            ++chunkId;
        } while (chunkId < header.ChunkNum);

        spdlog::info("--- end ---");
        return true;
    }
}