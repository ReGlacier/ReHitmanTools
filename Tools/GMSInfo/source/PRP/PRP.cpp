#include <PRP/PRP.h>
#include <PRP/PRPTypes.h>
#include <PRP/PRPVFSM.h>

#include <LevelAssets.h>
#include <LevelContainer.h>
#include <GlacierTypeDefs.h>
#include <TypesDataBase.h>

#include <BinaryWalker.h>
#include <BinaryWalkerADL.h>

#include <spdlog/spdlog.h>

#include <utility>
#include <fstream>
#include <algorithm>
#include <optional>

namespace ReGlacier
{
    static constexpr const char* kNoTag = "(NOT A TAG)";
    static constexpr const char* kExpectedIdentifier = "IOPacked v0.1";

    static constexpr int kKeysOffset = 0x17;
    static constexpr int kKeysListOffset = 0x1F;

    PRP_ETag PRP_ETag_Helpers::FromByte(uint8_t byte)
    {
        if (byte < PRP_ETag::TAG_Array || byte > PRP_ETag::TAG_NameBitfield)
        {
            return PRP_ETag::NO_TAG;
        }

        if (byte == 14 || byte == 128 || byte == 142 || (byte >= 16 && byte <= 123))
        {
            return PRP_ETag::TAG_UNKNOWN;
        }

        return static_cast<PRP_ETag>(byte);
    }

    std::string_view PRP_ETag_Helpers::ToString(PRP_ETag tag)
    {
#define RETAG(id) if (tag == (id)) { return #id; }
        RETAG(PRP_ETag::TAG_Array);
        RETAG(PRP_ETag::TAG_BeginObject);
        RETAG(PRP_ETag::TAG_Reference);
        RETAG(PRP_ETag::TAG_Container);
        RETAG(PRP_ETag::TAG_Char);
        RETAG(PRP_ETag::TAG_Bool);
        RETAG(PRP_ETag::TAG_Int8);
        RETAG(PRP_ETag::TAG_Int16);
        RETAG(PRP_ETag::TAG_Int32);
        RETAG(PRP_ETag::TAG_Float32);
        RETAG(PRP_ETag::TAG_Float64);
        RETAG(PRP_ETag::TAG_String);
        RETAG(PRP_ETag::TAG_RawData);
        RETAG(PRP_ETag::TAG_Bitfield);
        RETAG(PRP_ETag::TAG_EndArray);
        RETAG(PRP_ETag::TAG_SkipMark);
        RETAG(PRP_ETag::TAG_EndObject);
        RETAG(PRP_ETag::TAG_EndOfStream);
        RETAG(PRP_ETag::TAG_NamedArray);
        RETAG(PRP_ETag::TAG_BeginNamedObject);
        RETAG(PRP_ETag::TAG_NamedReference);
        RETAG(PRP_ETag::TAG_NamedContainer);
        RETAG(PRP_ETag::TAG_NamedChar);
        RETAG(PRP_ETag::TAG_NamedBool);
        RETAG(PRP_ETag::TAG_NamedInt8);
        RETAG(PRP_ETag::TAG_NamedInt16);
        RETAG(PRP_ETag::TAG_NamedInt32);
        RETAG(PRP_ETag::TAG_NamedFloat32);
        RETAG(PRP_ETag::TAG_NamedFloat64);
        RETAG(PRP_ETag::TAG_NamedString);
        RETAG(PRP_ETag::TAG_NamedRawData);
        RETAG(PRP_ETag::TAG_NameBitfield);
        RETAG(PRP_ETag::TAG_UNKNOWN);
        return kNoTag;
#undef RETAG
    }

    bool PRP_ETag_Helpers::IsTagIncreaseDepthLevel(PRP_ETag tag)
    {
        return
            tag == PRP_ETag::TAG_Array || tag == PRP_ETag::TAG_BeginObject ||
            tag == PRP_ETag::TAG_Container || tag == PRP_ETag::TAG_NamedContainer ||
            tag == PRP_ETag::TAG_BeginNamedObject || tag == PRP_ETag::TAG_NamedArray;
    }

    bool PRP_ETag_Helpers::IsTagDecreateDepthLevel(PRP_ETag tag)
    {
        return tag == PRP_ETag::TAG_EndArray || tag == PRP_ETag::TAG_EndObject || tag == PRP_ETag::TAG_EndOfStream;
    }

    PRP::PRP(std::string name, LevelContainer* levelContainer, LevelAssets* levelAssets)
        : IGameEntity(name, levelContainer, levelAssets)
    {}

    PRP::~PRP()
    {
        delete m_tree;
        m_tree = nullptr;
    }

    bool PRP::Load()
    {
        size_t prpBufferSize = 0;
        auto prpBuffer = m_container->Read(m_name, prpBufferSize);
        if (!prpBuffer)
        {
            spdlog::error("PRP::Load| Failed to load asset {}", m_name);
            return false;
        }

        IPRPVisitor* visitor = new IPRPVisitor();
        PRPWalker walker { std::move(prpBuffer), prpBufferSize };
        walker.Prepare(visitor);

        delete visitor;

//        BinaryWalker binaryWalker { prpBuffer.get(), prpBufferSize };
//
//        {
//            std::string headerStr = binaryWalker.ReadZString(strlen(kExpectedIdentifier));
//
//            if (std::string_view(headerStr) != std::string_view(kExpectedIdentifier))
//            {
//                spdlog::error("PRP::Load| Failed to load PRP {}. Reason: bad header!", m_name);
//                return false;
//            }
//
//            spdlog::info("PRP::Load| Header is OK");
//            binaryWalker.Seek(kKeysOffset, IBaseStreamWalker::BEGIN);
//        }
//
//        {
//            m_keysCount = binaryWalker.ReadUInt32();
//            m_valuesOffset = binaryWalker.ReadUInt32();
//
//            if (m_valuesOffset >= prpBufferSize)
//            {
//                spdlog::error("PRP::Load| Failed to load PRP {}. Reason: Bad 'values' offset!", m_name);
//                m_keysCount = 0;
//                m_valuesOffset = 0;
//                return false;
//            }
//        }
//
//        spdlog::info("PRP::Load| Header looks OK.");
//        spdlog::info("PRP::Load| Total keys: {}", m_keysCount);
//
//        m_keys.clear();
//        m_keys.reserve(m_keysCount);
//
//        while (m_keys.size() != m_keysCount)
//        {
//            std::string val = binaryWalker.ReadZString();
//            if (!val.empty())
//            {
//                m_keys.push_back(val);
//            }
//        }
//
//        // Decompile it
//        TryToDecompileEntities(std::move(prpBuffer), prpBufferSize);

        return true;
    }

    void PRP::PrintInfo()
    {
//        spdlog::info("PRP Info: ");
//        spdlog::info("Keys: {}", m_keysCount);
//        int index = 0;
//        for (index = 0; index < m_keysCount; index++)
//        {
//            spdlog::info(" [{}] = {}", index, m_keys[index]);
//        }
//        spdlog::info(" --- END OF PRP --- ");
    }

    void PRP::TryToDecompileEntities(std::unique_ptr<uint8_t[]>&& buffer, size_t bufferSize)
    {
//        BinaryWalker binaryWalker { buffer.get(), bufferSize };
//        binaryWalker.Seek(kKeysListOffset, IBaseStreamWalker::BEGIN);
//        binaryWalker.Seek(m_valuesOffset, IBaseStreamWalker::CURR);
//        spdlog::info("Current pos: {}", binaryWalker.GetPosition());

//        // Iterate over bytes, try to find TAG, if tag occured - decompress it's data if we can
//        PRP_ETag tag = PRP_ETag::NO_TAG;
//        int tagId = 0;
//
//        if (m_tree)
//        {
//            delete m_tree;
//            m_tree = nullptr;
//        }
//
//        m_tree = new PRPTreeNode(nullptr, PRP_ETag::NO_TAG); // No tag at root
//        auto currentNode = m_tree;
//
//        do
//        {
//            auto pos = binaryWalker.GetPosition();
//            uint8_t byte = binaryWalker.ReadUInt8();
//            tag = PRP_ETag_Helpers::FromByte(byte);
//
//            if (tag == PRP_ETag::NO_TAG) continue;
//            const auto tagName = PRP_ETag_Helpers::ToString(tag);
//
//            // Recognize tag flags
//            if (PRP_ETag_Helpers::IsTagIncreaseDepthLevel(tag))
//            {
//                // Allocate new depth level, insert as child, set current not to new depth controller
//                auto newChild = new PRPTreeNode(currentNode, tag);
//                currentNode->AddChild(newChild);
//                newChild->SetRefToBuffer(pos);
//                currentNode = newChild;
//            }
//            else if (PRP_ETag_Helpers::IsTagDecreateDepthLevel(tag))
//            {
//                // Change depth level of tree - jump to parent node
//                if (!currentNode->IsRoot())
//                {
//                    auto lastChild = new PRPTreeNode(currentNode, tag);
//                    lastChild->SetRefToBuffer(pos);
//                    currentNode->AddChild(lastChild);
//                    currentNode = currentNode->GetParent();
//                }
//                else
//                {
//                    spdlog::warn(" *** TREE TRAVELING WARNING: TAG {} AT +{:X} TOLD US THAT WE NEED TO JUMP UP, BUT NO WAY TO DO THIS! *** ", PRP_ETag_Helpers::ToString(tag), pos);
//                }
//            }
//            else
//            {
//                // Normal tag, insert as child
//                auto child = new PRPTreeNode(currentNode, tag);
//                child->SetRefToBuffer(pos);
//                currentNode->AddChild(child);
//            }
//
//            if (tag == PRP_ETag::TAG_Float32)
//            {
//                auto fvl = binaryWalker.Read<float>();
//                spdlog::info("[{}] +{:X} {} = {}", tagId, pos, tagName, fvl);
//            }
//            else if (tag == PRP_ETag::TAG_Int32)
//            {
//                auto i32 = binaryWalker.Read<int32_t>();
//                spdlog::info("[{}] +{:X} {} = {} (hex 0x{:08X})", tagId, pos, tagName, i32, i32);
//            }
//            else
//            {
//                spdlog::info("[{}] +{:X} {} = ?", tagId, pos, tagName);
//            }
//
//            ++tagId;
//        }
//        while (tag != PRP_ETag::TAG_EndOfStream);
//
//        spdlog::info("Total ETags: {}, Total Keys: {}", tagId + 1, m_keysCount);
    }
}
