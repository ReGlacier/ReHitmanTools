#include <cassert>
#include <sstream>
#include <iomanip>
#include <string_view>

#include <spdlog/spdlog.h>

#include <GMS/GMSTypes.h>
#include <GMS/GMS.h>

#include <PRP/PRPVFSM.h>

#include <BinaryWalkerADL.h>
#include <LevelContainer.h>
#include <LevelAssets.h>

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

    void PRPWalker::Prepare(IPRPVisitor* visitor, LevelContainer* pLevelContainer, LevelAssets* pAssets)
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
        while (tokenIndex <= m_header.totalKeysCount)
        {
            std::string token;
            BinaryWalkerADL<std::string>::Read(m_walker, token);
            /**
             * @brief We should save each available token.
             *        Empty tokens are valid too.
             *        Ref to ZTokenTable_Serializerlib::Load
             */

            m_tokenTable.emplace_back(std::move(token));
            ++tokenIndex;
        }

        spdlog::info("Tokens table is ready. Total tokens count is {}", m_header.totalKeysCount);
        m_walker.Seek(sizeof(SPRP) + m_header.dataOffset, BinaryWalker::BEGIN); //Reset position
        m_walker.Seek(4, BinaryWalker::CURR); //HACK! TODO: Find a way to fix this

        m_zDefines.clear();

        LoadZDefines(visitor);
        LoadProperties(visitor, pLevelContainer, pAssets);
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

    class PRPWalkController : public IPRPVisitor
    {
        enum ContextFlag : unsigned int
        {
            CF_NONE      = 0,
            CF_EOS       = 1 << 0,
            CF_ERROR     = 1 << 1,
            CF_OBJECT    = 1 << 2,
            CF_ARRAY     = 1 << 3,
            CF_CONTAINER = 1 << 4
        };

        unsigned int m_contextFlags { CF_NONE };

    public:
        void OnByte(PRPWalker* walker, uint8_t byte) override
        {
            if (IsEndOfStream()) return;

            const size_t offset = walker->GetCurrentOffset() - 1;
            const PRP_ETag tag = PRP_ETag_Helpers::FromByte(byte);


            switch (tag) {
                case TAG_Array:
                case TAG_NamedArray:
                {
                    m_contextFlags |= ContextFlag::CF_ARRAY;

                    uint32_t capacity = 0;

                    Visit_I32(walker, ZToken::Void, capacity);
                    assert(capacity > 0);

                    spdlog::info("[OP| +{:0X}] Begin array (size {}, opcode {:2X})", offset, capacity, byte);
                }
                break;
                case TAG_BeginObject:
                case TAG_BeginNamedObject:
                    m_contextFlags |= ContextFlag::CF_OBJECT;
                    spdlog::info("[OP| +{:0X}] Begin object (opcode {:2X})", offset, byte);
                    break;
                case TAG_Container:
                case TAG_NamedContainer:
                {
                    m_contextFlags |= ContextFlag::CF_CONTAINER;
                    uint32_t v = 0;
                    Visit_I32(walker, ZToken::Void, v);
                    spdlog::info("[OP| +{:0X}] Begin container <{}> (opcode {:2X})", offset, v, byte);
                }
                break;
                case TAG_EndArray:
                    m_contextFlags &= ~ContextFlag::CF_ARRAY;
                    spdlog::info("[OP| +{:0X}] End array", offset);
                    break;
                case TAG_EndObject:
                    m_contextFlags &= ~(ContextFlag::CF_OBJECT | ContextFlag::CF_CONTAINER);
                    spdlog::info("[OP| +{:0X}] End object", offset);
                    break;
                case TAG_EndOfStream:
                    m_contextFlags |= ContextFlag::CF_EOS;
                    spdlog::info("[OP| +{:0X}] >>> END OF STREAM <<<", offset);
                    break;

                case TAG_NamedReference:
                case TAG_Reference:
                    spdlog::info("[OP| +{:0X}] Reference (opcode {:2X})", offset, byte);
                    break;
                case TAG_Char:
                case TAG_NamedChar:
                {
                    uint8_t v = 0;
                    Visit_I8(walker, ZToken::Void, v);
                    spdlog::info("[OP| +{:0X}] Char {:c} (opcode {:2x})", offset, v, byte);
                }
                break;
                case TAG_Bool:
                case TAG_NamedBool:
                {
                    uint8_t v = 0;
                    Visit_I8(walker, ZToken::Void, v);
                    spdlog::info("[OP| +{:0X}] Bool {} (opcode {:2X})", offset, v, byte);
                }
                break;
                case TAG_Int8:
                case TAG_NamedInt8:
                {
                    uint8_t v = 0;
                    Visit_I8(walker, ZToken::Void, v);
                    spdlog::info("[OP| +{:0X}] Int8 {} (opcode {:2X})", offset, v, byte);
                }
                break;
                case TAG_Int16:
                case TAG_NamedInt16:
                {
                    uint16_t v = 0;
                    Visit_I16(walker, ZToken::Void, v);
                    spdlog::info("[OP| +{:0X}] Int16 {} (opcode {:2X})", offset, v, byte);
                }
                break;
                case TAG_Int32:
                case TAG_NamedInt32:
                {
                    uint32_t v = 0;
                    Visit_I32(walker, ZToken::Void, v);
                    spdlog::info("[OP| +{:0X}] Int32 {} (opcode {:2X})", offset, v, byte);
                }
                break;
                case TAG_Float32:
                case TAG_NamedFloat32:
                {
                    float v = .0f;
                    Visit_F32(walker, ZToken::Void, v);
                    spdlog::info("[OP| +{:0X}] Float32 {} (opcode {:2X})", offset, v, byte);
                }
                break;
                case TAG_Float64:
                case TAG_NamedFloat64:
                {
                    double v = .0f;
                    Visit_F64(walker, ZToken::Void, v);
                    spdlog::info("[OP| +{:0X}] Float32 {} (opcode {:2X})", offset, v, byte);
                }
                break;
                case TAG_String:
                case TAG_NamedString:
                {
                    std::string v = {};
                    Visit_String(walker, ZToken::Void, v);
                    spdlog::info("[OP| +{:0X}] String: \"{}\" (opcode {:2X})", offset, v, byte);
                }
                break;
                case TAG_RawData:
                case TAG_NamedRawData:
                {
                    size_t bufferSize { 0 };
                    std::shared_ptr<uint8_t[]> buffer;

                    Visit_RawBuffer(walker, ZToken::Void, buffer, bufferSize);

                    if (buffer && bufferSize)
                    {
                        spdlog::info("[OP| +{:0X}] TAG_RawData (opcode {:2X}). Buffer of size {:08X} bytes", offset, byte, bufferSize);
                        std::stringstream ss;

                        for (size_t i = 0; i < bufferSize; i++)
                        {
                            if (i != 0 && i % 8 == 0) {
                                ss << '\n';
                            }
                            else
                            {
                                ss << std::hex << std::setfill('0') << std::setw(2) << static_cast<int>(buffer.get()[i]) << ' ';
                            }
                        }
                        spdlog::info("{}", ss.str());
                    }
                    else
                    {
                        spdlog::error("[OP| +{:0X}] TAG_RawData (opcode {:2X}). Bad buffer!", offset, byte);
                        assert(false);
                    }
                }
                break;
                case TAG_Bitfield:
                case TAG_NameBitfield:
                {
                    uint32_t v { 0 };
                    Visit_I32(walker, ZToken::Void, v);
                    spdlog::info("[OP| +{:0X}] TAG_Bitfield: {} (opcode {:2X})", offset, v, byte);
                }
                break;
                case TAG_SkipMark:
                    spdlog::info("[OP| +{:0X}] TAG_SkipMark. Just skip current opcode", offset);
                    break;
                /// >>> MY TAGS <<<
                case TAG_StringOrArray_E:
                case TAG_StringOrArray_8E:
                {
                    if ((walker->GetHeaderBFlags() >> 2) & 1) {
                        std::string s {};
                        Visit_String(walker, ZToken::Void, s);
                        spdlog::info("[OP| +{:0X}] TAG_StringOrArray. Read as string: {} (opcode {:2X})", offset, s, byte);
                    }
                    else
                    {
                        uint32_t v { 0 };
                        Visit_I32(walker, ZToken::Void, v);
                        spdlog::info("[OP| +{:0X}] TAG_StringOrArray. Read as array: {} (i32, opcode {:2X})", offset, v, byte);
                    }
                }
                break;
                case TAG_StringArray:
                {
                    if ((walker->GetHeaderBFlags() >> 2) & 1)
                    {
                        if ((walker->GetHeaderBFlags() >> 3) & 1)
                        {
                            //Impl #1
                            uint32_t capacity { 0 };
                            Visit_I32(walker, ZToken::Void, capacity);

                            if (capacity == 0) {
                                spdlog::info("[OP| +{:X}] TAG_StringArray (impl 1, opcode {:2X}). Array of strings: (EMPTY)", offset, byte);
                            }
                            else {
                                spdlog::info("[OP| +{:X}] TAG_StringArray (impl 1, opcode {:2X}). Array of strings ({}): ", offset, byte, capacity);

                                for (int i = 0; i < capacity; i++)
                                {
                                    std::string s;
                                    Visit_String(walker, ZToken::Void, s);

                                    spdlog::info("\t[{}] = \"{}\"", i, s);
                                }
                            }
                        }
                        else
                        {
                            //Impl #2 (at 0x458CD0)
                            spdlog::error("[OP| +{:X}] TAG_StringArray (impl 2, opcode {:2X}). Not implemented", offset, byte);
                            assert(false);
//                            uint32_t v1 { 0 };
//                            Visit_I32(walker, ZToken::Void, v1); ///???????????
//
//                            uint32_t capacity { 0 };
//                            Visit_I32(walker, ZToken::Void, capacity);
//
//                            if (capacity > 0)
//                            {
//                                uint32_t v2 { 0 };
//                                Visit_I32(walker, ZToken::Void, v2);
//
//                            }
                        }
                    }
                    else
                    {
                        uint32_t v { 0 };
                        Visit_I32(walker, ZToken::Void, v);
                        spdlog::info("[OP| +{:0X}] TAG_StringArray (impl 3, opcode {:2X}). Array as I32: {}", offset, byte, v);
                    }
                }
                break;
                /// ETC
                case TAG_UNKNOWN:
                {
                    uint32_t v = 0;
                    Visit_I32(walker, ZToken::Void, v);
                    spdlog::warn("[OP| +{:0X}] TAK_UNKNOWN. Read next I32 {} (opcode {:2X})", offset, v, byte);
                }
                break;
                case NO_TAG:
                default:
                {
                    m_contextFlags |= ContextFlag::CF_ERROR;
                    spdlog::error("[OP| +{:X}] Unknown opcode {:2X}", offset, byte);
                    assert(false);
                }
                break;
            }
        }

        void Reset()
        {
            m_contextFlags = ContextFlag::CF_NONE;
        }

        [[nodiscard]] bool IsInObject() const {
            return m_contextFlags & ContextFlag::CF_OBJECT;
        }

        [[nodiscard]] bool IsInArray() const {
            return m_contextFlags & ContextFlag::CF_ARRAY;
        }

        [[nodiscard]] bool IsInContainer() const {
            return m_contextFlags & ContextFlag::CF_CONTAINER;
        }

        [[nodiscard]] bool IsEndOfStream() const {
            return m_contextFlags & ContextFlag::CF_EOS;
        }

        [[nodiscard]] bool IsError() const {
            return m_contextFlags & ContextFlag::CF_ERROR;
        }
    };

    bool PRPWalker::LoadProperties(IPRPVisitor* visitor, LevelContainer* pLevel, LevelAssets* pAssets)
    {
        /**
         * @note this is test code, please refactor this
         * @todo REFACTOR THIS LATER
         */
        auto gms = std::make_unique<GMS>(pAssets->GMS, pLevel, pAssets);
        if (!gms->Load())
        {
            spdlog::error("PRPWalker| Failed to load properties (unable to load GMS)");
            return false;
        }

        auto geoms = gms->GetGeoms();
        PRPWalkController controller {};

        do {
            assert(!controller.IsError());
            controller.OnByte(this, m_walker.ReadUInt8());
        } while (!controller.IsEndOfStream());

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
        if (((walker->GetHeaderBFlags() >> 3) & 1) == 0) {
            uint32_t length { 0 };
            Visit_I32(walker, token, length);

            assert(length > 0);

            result.resize(length + 1);
            result[length] = 0x0;

            Visit_RawContents(walker, token, result.data(), length);
        }
        else
        {
            const uint32_t index = walker->m_walker.ReadUInt32();
            if (index >= walker->m_tokenTable.size()) {
                spdlog::error("IPRPVisitor::Visit_String| Index of string is out of bounds! Index is {:8X}", index);
                assert(false);
                return;
            }

            result = walker->m_tokenTable[index];
        }
    }

    void IPRPVisitor::Visit_RawBuffer(PRPWalker* walker,
                                      PRPToken token,
                                      std::shared_ptr<uint8_t[]>& buffer,
                                      size_t& bufferSize) {
        uint32_t size { 0 };

        Visit_I32(walker, token, size);
        if (size) {
            bufferSize = size;

            buffer = std::make_shared<uint8_t[]>(size);
            walker->m_walker.ReadArray(buffer.get(), bufferSize);
        }
    }

    void IPRPVisitor::Visit_RawContents(PRPWalker* walker, PRPToken token, char* buffer, size_t bufferSize)
    {
        assert(bufferSize > 0);
        walker->m_walker.ReadArray(buffer, bufferSize);
    }

    void IPRPVisitor::Visit_EndOfStream(PRPWalker* walker)
    {
        walker->m_flags |= PRPWalker::Flags::END_OF_STREAM;
        spdlog::info("IPRPVisitor| End of stream");
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