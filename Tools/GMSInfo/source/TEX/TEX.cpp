#include <TEX/TEX.h>
#include <TEX/TEXTypes.h>
#include <TEX/ADL/TEXADL.h>

#include <GlacierTypeDefs.h>
#include <LevelContainer.h>
#include <TypesDataBase.h>
#include <LevelAssets.h>

#include <spdlog/spdlog.h>

#include <BinaryWalker.h>
#include <BinaryWalkerADL.h>

#include <utility>
#include <array>

namespace ReGlacier
{
    static constexpr size_t kOffsetsTableSize = 0x800;
    static constexpr size_t kBadOffset        = 0x0;

    TEX::TEX(std::string name, LevelContainer* levelContainer, LevelAssets* levelAssets)
        : IGameEntity(name, levelContainer, levelAssets)
    {}

    bool TEX::Load()
    {
        size_t texBufferSize = 0;
        auto texBuffer = m_container->Read(m_name, texBufferSize);
        if (!texBuffer)
        {
            spdlog::error("TEX::Load| Failed to load TEX file {}", m_name);
            return false;
        }

        BinaryWalker binaryWalker(std::move(texBuffer), texBufferSize);

        STEXHeader header {};
        BinaryWalkerADL<STEXHeader>::Read(binaryWalker, header);

        binaryWalker.Seek(header.Table1Location, BinaryWalker::SeekType::FROM_BEGIN);

        std::array<uint32_t, kOffsetsTableSize> offsetsTable {};
        binaryWalker.ReadArray(offsetsTable);

        for (size_t i = 0; i < kOffsetsTableSize; i++)
        {
            const auto offset = offsetsTable[i];

            if (offset == kBadOffset)
                continue;

            binaryWalker.Seek(offset, BinaryWalker::SeekType::FROM_BEGIN);
            STEXEntry entry {};
            BinaryWalkerADL<STEXEntry>::Read(binaryWalker, entry);

            spdlog::info("Break here");
        }

        texBuffer = std::move(binaryWalker.Take());
        spdlog::info("--- END ---");
        return true;
    }
}