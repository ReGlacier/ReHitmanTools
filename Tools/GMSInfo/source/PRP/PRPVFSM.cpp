#include <PRP/PRPVFSM.h>
#include <spdlog/spdlog.h>
#include <cassert>
#include <string_view>
#include <BinaryWalkerADL.h>

namespace ReGlacier
{
    template <>
    struct BinaryWalkerADL<SPRP>
    {
        static void Read(const BinaryWalker& binaryWalker, SPRP& value)
        {
            binaryWalker.ReadArray<char>(&value.magic[0], SPRP::kMagicSize);
            value.bIsRaw = static_cast<bool>(binaryWalker.ReadUInt8());
            value.bFlags = binaryWalker.ReadUInt32();
            value.__Unknown__ = binaryWalker.ReadUInt32();
            value.totalKeysCount = binaryWalker.ReadUInt32();
            value.dataOffset = binaryWalker.ReadUInt32();
        }

        static void Write(BinaryWalker& binaryWalker, const SPRP& value)
        {
            spdlog::error("NOT IMPLEMENTED");
            assert(false);
        }
    };

    static constexpr const char* kExpectedIdentifier = "IOPacked v0.1";
    static constexpr int kKeysOffset = 0x17;

    PRPWalker::PRPWalker(std::unique_ptr<uint8_t[]>&& buffer, size_t bufferSize)
        : m_buffer(std::move(buffer))
        , m_bufferSize(bufferSize)
        , m_flags(Flags::NO_FLAG)
    {
    }

    void PRPWalker::Prepare(IPRPVisitor* visitor)
    {
        if (!visitor) {
            assert(visitor != nullptr);
            return;
        }

        m_walker = BinaryWalker { m_buffer.get(), m_bufferSize };
        BinaryWalkerADL<SPRP>::Read(m_walker, m_header);

        if (std::string_view { m_header.magic } != std::string_view { kExpectedIdentifier }) {
            m_flags |= Flags::ERROR_IN_STREAM;
            spdlog::error("PRPWalker| Bad PRP file (magic mismatch!)");
            return;
        }

        if (m_header.totalKeysCount >= 0xDEADBEEF) {
            m_flags |= Flags::ERROR_IN_STREAM;
            spdlog::error("PRPWalker| Bad PRP file (too much tokens in table");
            return;
        }

        if (m_header.dataOffset >= m_bufferSize) {
            m_flags |= Flags::ERROR_IN_STREAM;
            spdlog::error("PRPWalker| Bad PRP file (bad data offset");
            return;
        }

        spdlog::info("PRP looks ok. Reading tokens table");
        m_tokenTable.clear();

        m_tokenTable.reserve(m_header.totalKeysCount);

        int tokenIndex = 0;
        while (tokenIndex < m_header.totalKeysCount)
        {
            std::string token;
            BinaryWalkerADL<std::string>::Read(m_walker, token);
            if (token.empty()) continue;

            m_tokenTable.emplace_back(std::move(token));
            ++tokenIndex;
        }

        spdlog::info("Tokens table is ready. Total tokens count is {}", m_header.totalKeysCount);
        m_walker.Seek(sizeof(SPRP) + m_header.dataOffset, BinaryWalker::BEGIN); //Reset position
        m_walker.Seek(4, BinaryWalker::CURR); //HACK! TODO: Find a way to fix this

        m_zDefines.clear();

        LoadZDefines(visitor);
        spdlog::info("DBG");
    }

    bool PRPWalker::LoadZDefines(IPRPVisitor* visitor)
    {
        // Load ZDefines
        uint32_t capacity { 0 };

        visitor->ExchangeContainer(this, &capacity);

        for (int i = 0; i < capacity; i++)
        {
            std::string name;
            uint32_t typeId { 0 };

            /// Get Name
            visitor->ExchangeHeader(this, EPropertyType::Type_2 | EPropertyType::Type_8 | EPropertyType::Type_1);
            visitor->Visit_String(this, ZToken::Void, name);
            visitor->ExchangeFooter(this, EPropertyType::Type_2 | EPropertyType::Type_8 | EPropertyType::Type_1);

            /// Get Type ID
            visitor->ExchangeHeader(this, EPropertyType::Type_8);
            visitor->Visit_I32(this, ZToken::Void, typeId);
            visitor->ExchangeFooter(this, EPropertyType::Type_8);

            ZDefine_t value;

            /// Read internal data
            switch (typeId)
            {
                case 2:
                {
                    uint32_t size { 0 };

                    // Read Size
                    visitor->ExchangeHeader(this, EPropertyType::Type_8);
                    visitor->Visit_I32(this, ZToken::Void, size);
                    visitor->ExchangeFooter(this, EPropertyType::Type_8);

                    // Read Data
                    ZI32Vec vec;
                    vec.resize(size);
                    visitor->ExchangeArray<uint32_t>(this, ZToken::Void, vec.data(), size);

                    value = vec;
                }
                    break;
                case 3:
                {
                    uint32_t size { 0 };

                    // Read Size
                    visitor->ExchangeHeader(this, EPropertyType::Type_8);
                    visitor->Visit_I32(this, ZToken::Void, size);
                    visitor->ExchangeFooter(this, EPropertyType::Type_8);

                    // Read Data
                    ZF32Vec vec;
                    vec.resize(size);
                    visitor->ExchangeArray<float>(this, ZToken::Void, vec.data(), size);

                    value = vec;
                }
                    break;
                case 0xC:
                case 0xE:
                case 0x10:
                {
                    std::string geomRefStr;

                    visitor->ExchangeHeader(this, EPropertyType::Type_2 | EPropertyType::Type_8 | EPropertyType::Type_1);
                    visitor->Visit_String(this, ZToken::Void, geomRefStr);
                    visitor->ExchangeFooter(this, EPropertyType::Type_2 | EPropertyType::Type_8 | EPropertyType::Type_1);

                    value = geomRefStr;
                }
                    break;
                case 0x11:
                {
                    unsigned int elementsNr { 0 };

                    visitor->ExchangeContainer(this, &elementsNr);
                    std::vector<std::string> reftab;
                    reftab.reserve(elementsNr);

                    for (int j = 0; j < elementsNr; j++)
                    {
                        std::string geomName;
                        visitor->ExchangeHeader(this, EPropertyType::Type_2 | EPropertyType::Type_8 | EPropertyType::Type_1);
                        visitor->Visit_String(this, ZToken::Void, geomName);
                        visitor->ExchangeFooter(this, EPropertyType::Type_2 | EPropertyType::Type_8 | EPropertyType::Type_1);

                        reftab.push_back(geomName);
                    }

                    value = reftab;
                }
                    break;
                default:
                    spdlog::warn("Unknown type id {}", typeId);
                    assert(false);
                    break;
            }

            m_zDefines[name] = value;
        }

        return true;
    }

    uint8_t PRPWalker::GetCurrentByte() const {
        return m_walker.ReadInt8();
    }

    size_t PRPWalker::GetCurrentOffset() const {
        return m_walker.GetPosition();
    }

    /// ---------------------------------------------------------------------------------------------------------

    void IPRPVisitor::ExchangeHeader(PRPWalker* walker, unsigned int /*type*/)
    {
        walker->m_walker.ReadUInt8();
    }

    void IPRPVisitor::ExchangeFooter(PRPWalker* walker, unsigned int type)
    {
        if (type == EPropertyType::Type_14) {
            if (walker->m_header.bFlags & 1) {
                walker->m_walker.ReadUInt8();
            }
        }
    }

    void IPRPVisitor::ExchangeContainer(PRPWalker* walker, unsigned int* capacity)
    {
        ExchangeHeader(walker, EPropertyType::Type_10);

        const uint32_t res = walker->m_walker.ReadUInt32();
        if (capacity) {
            *capacity = res;
        }
    }

    void IPRPVisitor::BeginArray(PRPWalker* walker, PRPToken token, unsigned int size)
    {
        if (!(walker->m_header.bFlags & 1))
            return;

        walker->m_walker.ReadUInt8();

        if (walker->m_header.bFlags & 1)
        {
            walker->m_walker.ReadUInt32();
            //TODO: Maybe we should check size constrains here? (walker->m_walker.ReadUInt32() == size)
        }
    }

    void IPRPVisitor::EndArray(PRPWalker* walker)
    {
        walker->m_walker.ReadUInt8();
    }

    void IPRPVisitor::Visit_I8(PRPWalker* walker, PRPToken token, uint8_t& result)
    {
        result = walker->GetCurrentByte();
    }

    void IPRPVisitor::Visit_I16(PRPWalker* walker, PRPToken token, uint16_t& result)
    {
        result = walker->m_walker.ReadUInt16();
    }

    void IPRPVisitor::Visit_I32(PRPWalker* walker, PRPToken token, uint32_t& result)
    {
        result = walker->m_walker.ReadUInt32();
    }

    void IPRPVisitor::Visit_F32(PRPWalker* walker, PRPToken token, float& result)
    {
        result = walker->m_walker.Read<float>();
    }

    void IPRPVisitor::Visit_F64(PRPWalker* walker, PRPToken token, double& result)
    {
        result = walker->m_walker.Read<double>();
    }

    void IPRPVisitor::Visit_String(PRPWalker* walker, PRPToken token, std::string& result)
    {
        const uint32_t index = walker->m_walker.ReadUInt32();
        if (index > walker->m_tokenTable.size()) {
            spdlog::error("IPRPVisitor::Visit_String| Index of string is out of bounds! Index is {:8X}", index);
            return;
        }

        result = walker->m_tokenTable[index];
    }

    void IPRPVisitor::Visit_EndOfStream(PRPWalker* walker)
    {
        walker->m_flags |= PRPWalker::Flags::END_OF_STREAM;
        spdlog::info("IPRPVisitor| End of stream");
    }

    void IPRPVisitor::Visit_Unrecognized(PRPWalker* walker, size_t offset, uint8_t byte)
    {
        spdlog::warn("IPRPVisitor| Unrecognized byte of context. Byte {:2X} at +0x{:8X}", byte, offset);
    }

    template<>
    void IPRPVisitor::ExchangeArray<uint32_t>(PRPWalker* walker, PRPToken token, uint32_t* data, unsigned int size)
    {
        BeginArray(walker, token, size);

        auto sz = size;
        uint32_t* pCurrent = data;
        do {
            ExchangeHeader(walker, EPropertyType::Type_7);
            Visit_I32(walker, token, reinterpret_cast<uint32_t&>(*pCurrent));
            ExchangeFooter(walker, EPropertyType::Type_7);

            ++pCurrent;
            --sz;
        } while (sz);

        EndArray(walker);
    }

    template<>
    void IPRPVisitor::ExchangeArray<float>(PRPWalker* walker, PRPToken token, float* data, unsigned int size)
    {
        BeginArray(walker, token, size);

        auto sz = size;
        float* pCurrent = data;
        do {
            ExchangeHeader(walker, EPropertyType::Type_7);
            Visit_F32(walker, token, reinterpret_cast<float&>(*pCurrent));
            ExchangeFooter(walker, EPropertyType::Type_7);

            ++pCurrent;
            --sz;
        } while (sz);

        EndArray(walker);
    }
}