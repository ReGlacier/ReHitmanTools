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
        m_textures.clear();

        size_t texBufferSize = 0;
        auto texBuffer = m_container->Read(m_name, texBufferSize);
        if (!texBuffer)
        {
            spdlog::error("TEX::Load| Failed to load TEX file {}", m_name);
            return false;
        }

        BinaryWalker binaryWalker(texBuffer.get(), texBufferSize);

        STEXHeader header {};
        BinaryWalkerADL<STEXHeader>::Read(binaryWalker, header);

        binaryWalker.Seek(header.Table1Location, BinaryWalker::SeekType::FROM_BEGIN);

        std::array<uint32_t, kOffsetsTableSize> offsetsTable {};
        binaryWalker.ReadArray(offsetsTable);

        int emptyBlocks = 0;

        for (size_t i = 0; i < kOffsetsTableSize; i++)
        {
            const auto offset = offsetsTable[i];

            if (offset == kBadOffset)
            {
                if (m_textures.empty())
                {
                    ++emptyBlocks;
                }
                continue;;
            }

            binaryWalker.Seek(offset, BinaryWalker::SeekType::FROM_BEGIN);
            STEXEntry entry {};
            BinaryWalkerADL<STEXEntry>::Read(binaryWalker, entry);

            auto texture = std::make_shared<Texture>();
            texture->m_allocationInfoPool.reserve(entry.MipMapLevels);
            texture->m_width = entry.Width;
            texture->m_height = entry.Height;
            texture->m_mipLevel = entry.MipMapLevels;
            texture->m_gameTexType = entry.Type;
            texture->m_name = entry.FileName;
            texture->m_unknown2 = entry.Unknown2;

            for (size_t j = 0; j < entry.MipMapLevels; j++)
            {
                auto& allocationInfo = texture->m_allocationInfoPool.emplace_back();
                BinaryWalkerADL<STEXEntityAllocationInfo>::Read(binaryWalker, allocationInfo);
            }

            if (entry.Type == ETEXEntityType::BITMAP_PAL)
            {
                texture->m_PALPaletteData = SPALPaletteInfo();
                auto& value = texture->m_PALPaletteData.value();
                value.Size = binaryWalker.Read<int32_t>();
                value.DataSize = texture->m_PALPaletteData.value().Size * 4;
                value.Data = std::make_unique<char[]>(value.DataSize);
                binaryWalker.ReadArray(value.Data.get(), value.DataSize);
            }

            m_textures.push_back(texture);
        }

        std::array<uint32_t, kOffsetsTableSize> offsetsTable2 {};
        binaryWalker.Seek(header.Table2Location, BinaryWalker::SeekType::FROM_BEGIN);

        for (int i = 0; i < kOffsetsTableSize; i++)
        {
            offsetsTable2[i] = binaryWalker.Read<uint32_t>();
            auto position = binaryWalker.GetPosition();

            if (offsetsTable2[i] != 0)
            {
                binaryWalker.Seek(offsetsTable2[i], BinaryWalker::SeekType::FROM_BEGIN);

                auto indicesCount = binaryWalker.Read<int32_t>();
                std::vector<int32_t> indices;
                indices.reserve(indicesCount);

                // TODO: Rewrite to ReadArray
                for (int j = 0; j < indicesCount; j++)
                {
                    indices.push_back(binaryWalker.Read<int32_t>());
                }

                int index = 0;

                if (indices[indicesCount - 1] == 0)
                {
                    for (int j = indicesCount - 1; j >= 0; j--)
                    {
                        if (indices[j] > 0)
                        {
                            index = indices[j] - emptyBlocks;
                            break;
                        }
                    }
                } else {
                    index = indices[indicesCount - 1] - emptyBlocks;
                }

                assert(index >= 0 && index < m_textures.size());

                m_textures[index]->m_indicesCount = indicesCount;
                m_textures[index]->m_indices.reserve(indicesCount);

                std::copy(std::begin(indices), std::end(indices), std::back_inserter(m_textures[index]->m_indices));

                binaryWalker.Seek(position, BinaryWalker::SeekType::FROM_BEGIN);
            }
        }

        spdlog::info("TEX::Load| Total textures in memory: {}", m_textures.size());
        return true;
    }

    const std::vector<Texture::Ptr> & TEX::GetLoadedTextures() const
    {
        return m_textures;
    }
}