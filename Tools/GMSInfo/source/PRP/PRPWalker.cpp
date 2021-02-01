#include <cassert>
#include <sstream>
#include <iomanip>
#include <string_view>

#include <spdlog/spdlog.h>

#include <PRP/IPRPVisitor.h>

#include <BinaryWalkerADL.h>

#include <PRP/PRPWalker.h>
#include <PRP/PRPADL.h>

namespace ReGlacier
{
    static constexpr const char* kExpectedIdentifier = "IOPacked v0.1";

    PRPWalker::PRPWalker(std::unique_ptr<uint8_t[]>&& buffer, size_t bufferSize)
        : m_buffer(std::move(buffer))
        , m_bufferSize(bufferSize)
    {
    }

    bool PRPWalker::Prepare(IPRPVisitor* visitor)
    {
        if (!visitor) {
            assert(visitor != nullptr);
            return false;
        }

        if (!ValidateHeader()) {
            spdlog::error("PRPWalker| Invalid PRP file");
            assert(false);
            return false;
        }

        spdlog::info("PRPWalker| PRP looks ok. Reading tokens table");
        m_tokenTable.clear();

        if (!LoadTokenTable())
        {
            spdlog::error("PRPWalker| Failed to load token table!");
            assert(false);
            return false;
        }

        spdlog::info("PRPWalker| Tokens table is ready. Total tokens count is {}", m_header.totalKeysCount);
        m_walker.Seek(sizeof(SPRP) + m_header.dataOffset, BinaryWalker::BEGIN); //Reset position
        m_totalObjectsPresented = m_walker.ReadUInt32();

        m_zDefines.clear();

        if (!LoadZDefines(visitor)) {
            spdlog::error("PRPWalker| Failed to load ZDefines section");
            assert(false);
            return false;
        }

        if (!LoadProperties(visitor)) {
            spdlog::error("PRPWalker| Failed to load properties section");
            assert(false);
            return false;
        }

        return true;
    }

    bool PRPWalker::ValidateHeader()
    {
        m_walker = BinaryWalker { m_buffer.get(), m_bufferSize };
        BinaryWalkerADL<SPRP>::Read(m_walker, m_header);

        if (std::string_view { m_header.magic } != std::string_view { kExpectedIdentifier }) {
            m_contextFlags |= ContextFlag::CF_ERROR;
            spdlog::error("PRPWalker| Bad PRP file (magic mismatch!)");
            return false;
        }

        if (m_header.totalKeysCount >= 0xDEADBEEF) {
            m_contextFlags |= ContextFlag::CF_ERROR;
            spdlog::error("PRPWalker| Bad PRP file (too much tokens in table");
            return false;
        }

        if (m_header.dataOffset >= m_bufferSize) {
            m_contextFlags |= ContextFlag::CF_ERROR;
            spdlog::error("PRPWalker| Bad PRP file (bad data offset");
            return false;
        }

        return true;
    }

    bool PRPWalker::LoadTokenTable()
    {
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

        return true;
    }

    bool PRPWalker::LoadZDefines(IPRPVisitor* visitor)
    {
        // Load ZDefines
        uint32_t capacity { 0 };

        ExchangeContainer(&capacity);

        for (int i = 0; i < capacity; i++)
        {
            std::string name;
            uint32_t typeId { 0 };

            /// Get Name
            ExchangeHeader(EPropertyType::Type_2 | EPropertyType::Type_8 | EPropertyType::Type_1);
            Exchange_String(name);
            ExchangeFooter(EPropertyType::Type_2 | EPropertyType::Type_8 | EPropertyType::Type_1);

            /// Get Type ID
            ExchangeHeader(EPropertyType::Type_8);
            Exchange_I32(typeId);
            ExchangeFooter(EPropertyType::Type_8);

            ZDefine_t value {};

            constexpr uint32_t kTypeId_Array_I32 = 2;
            constexpr uint32_t kTypeId_Array_F32 = 3;
            constexpr uint32_t kTypeId_StringRef_Type1 = 0xC;
            constexpr uint32_t kTypeId_StringRef_Type2 = 0xE;
            constexpr uint32_t kTypeId_StringRef_Type3 = 0x10;
            constexpr uint32_t kTypeId_StringREFTAB = 0x11;

            /// Read internal data
            switch (typeId)
            {
                case kTypeId_Array_I32:
                {
                    uint32_t size { 0 };

                    // Read Size
                    ExchangeHeader(EPropertyType::Type_8);
                    Exchange_I32(size);
                    ExchangeFooter(EPropertyType::Type_8);

                    // Read Data
                    ZI32Vec vec;
                    vec.resize(size);
                    ExchangeArray<uint32_t>(vec.data(), size);

                    value = vec;

                    spdlog::info("ZDefine| Array_I32 \"{}\" (length is {}):", name, size);
                    for (const auto& entry: vec) {
                        spdlog::info("ZDefine| Array_I32::Entry {}", entry);
                    }
                }
                break;
                case kTypeId_Array_F32:
                {
                    uint32_t size { 0 };

                    // Read Size
                    ExchangeHeader(EPropertyType::Type_8);
                    Exchange_I32(size);
                    ExchangeFooter(EPropertyType::Type_8);

                    // Read Data
                    ZF32Vec vec;
                    vec.resize(size);
                    ExchangeArray<float>(vec.data(), size);

                    value = vec;

                    spdlog::info("ZDefine| Array_F32 \"{}\" (length is {}):", name, size);
                    for (const auto& entry: vec) {
                        spdlog::info("ZDefine| Array_F32::Entry {:F}", entry);
                    }
                }
                break;
                case kTypeId_StringRef_Type1:
                case kTypeId_StringRef_Type2:
                case kTypeId_StringRef_Type3:
                {
                    std::string geomRefStr {};

                    ExchangeHeader(EPropertyType::Type_2 | EPropertyType::Type_8 | EPropertyType::Type_1);
                    Exchange_String(geomRefStr);
                    ExchangeFooter(EPropertyType::Type_2 | EPropertyType::Type_8 | EPropertyType::Type_1);

                    value = geomRefStr;

                    spdlog::info("ZDefine| STRREF ({:X}) {} = \"{}\"", typeId, name, geomRefStr);
                }
                break;
                case kTypeId_StringREFTAB:
                {
                    unsigned int elementsNr { 0 };

                    ExchangeContainer(&elementsNr);
                    std::vector<std::string> reftab;
                    reftab.reserve(elementsNr);

                    spdlog::info("ZDefine| REFTAB \"{}\" (length is {}):", name, elementsNr);

                    for (int j = 0; j < elementsNr; j++)
                    {
                        std::string geomName;
                        ExchangeHeader(EPropertyType::Type_2 | EPropertyType::Type_8 | EPropertyType::Type_1);
                        Exchange_String(geomName);
                        ExchangeFooter(EPropertyType::Type_2 | EPropertyType::Type_8 | EPropertyType::Type_1);

                        reftab.push_back(geomName);

                        spdlog::info("ZDefine| REFTAB::Entry \"{}\"", geomName);
                    }

                    value = reftab;
                }
                break;
                default:
                    spdlog::error("ZDefine| Unknown type id {}", typeId);
                    assert(false);
                    break;
            }

            m_zDefines[name] = value;
        }

        return true;
    }

    bool PRPWalker::LoadProperties(IPRPVisitor* visitor)
    {
        do {
            PrepareByte(visitor, m_walker.ReadUInt8());

            if (m_contextFlags & ContextFlag::CF_ERROR) {
                spdlog::error("PRPWalker| Got error on last prepare step. Check logs for details");
                assert(false);
                return false;
            }

        } while (!(m_contextFlags & ContextFlag::CF_EOS));

        return true;
    }

    void PRPWalker::PrepareByte(IPRPVisitor* visitor, uint8_t byte)
    {
        if (m_contextFlags & ContextFlag::CF_EOS) return;

        const size_t offset = GetCurrentOffset() - 1;
        const PRP_ETag tag = PRP_ETag_Helpers::FromByte(byte);

        switch (tag) {
            case TAG_Array:
            case TAG_NamedArray:
            {
                m_contextFlags |= ContextFlag::CF_ARRAY;

                uint32_t capacity = 0;

                Exchange_I32(capacity);
                if (capacity <= 0) {
                    m_contextFlags |= ContextFlag::CF_ERROR;
                    assert(capacity > 0);
                    return;
                }

                visitor->Visit_BeginArray(capacity);
            }
            break;
            case TAG_BeginObject:
            case TAG_BeginNamedObject:
                m_contextFlags |= ContextFlag::CF_OBJECT;
                visitor->Visit_BeginObject();
            break;
            case TAG_Container:
            case TAG_NamedContainer:
            {
                m_contextFlags |= ContextFlag::CF_CONTAINER;
                uint32_t v = 0;
                Exchange_I32(v);

                visitor->Visit_Container(v);
            }
            break;
            case TAG_EndArray:
                m_contextFlags &= ~ContextFlag::CF_ARRAY;
                visitor->Visit_EndArray();
            break;
            case TAG_EndObject:
                m_contextFlags &= ~(ContextFlag::CF_OBJECT | ContextFlag::CF_CONTAINER);
                visitor->Visit_EndObject();
            break;
            case TAG_EndOfStream:
                m_contextFlags |= ContextFlag::CF_EOS;
                visitor->Visit_EndOfStream();
            break;
            case TAG_NamedReference:
            case TAG_Reference:
                //TODO: Visit reference
                assert(false);
                spdlog::error("[OP| +{:0X}] Reference (opcode {:2X}) not supported", offset, byte);
            break;
            case TAG_Char:
            case TAG_NamedChar:
            {
                uint8_t v = 0;
                Exchange_I8(v);

                visitor->Visit_Char(v);
            }
            break;
            case TAG_Bool:
            case TAG_NamedBool:
            {
                uint8_t v = 0;
                Exchange_I8(v);
                visitor->Visit_Bool(static_cast<bool>(v));
            }
            break;
            case TAG_Int8:
            case TAG_NamedInt8:
            {
                uint8_t v = 0;
                Exchange_I8(v);
                visitor->Visit_I8(v);
            }
            break;
            case TAG_Int16:
            case TAG_NamedInt16:
            {
                uint16_t v = 0;
                Exchange_I16(v);
                visitor->Visit_I16(v);
            }
            break;
            case TAG_Int32:
            case TAG_NamedInt32:
            {
                uint32_t v = 0;
                Exchange_I32(v);
                visitor->Visit_I32(v);
            }
            break;
            case TAG_Float32:
            case TAG_NamedFloat32:
            {
                float v = .0f;
                Exchange_F32(v);
                visitor->Visit_F32(v);
            }
            break;
            case TAG_Float64:
            case TAG_NamedFloat64:
            {
                double v = .0f;
                Exchange_F64(v);
                visitor->Visit_F64(v);
            }
            break;
            case TAG_String:
            case TAG_NamedString:
            {
                std::string v = {};
                Exchange_String(v);
                visitor->Visit_String(v);
            }
            break;
            case TAG_RawData:
            case TAG_NamedRawData:
            {
                size_t bufferSize { 0 };
                std::shared_ptr<uint8_t[]> buffer;

                Exchange_RawBuffer(buffer, bufferSize);

                if (!bufferSize || !buffer) {
                    m_contextFlags |= ContextFlag::CF_ERROR;
                    spdlog::error("[OP| +{:0X}] TAG_RawData (opcode {:2X}). Bad buffer!", offset, byte);
                    assert(false);
                    return;
                }

                visitor->Visit_RawBuffer(buffer, bufferSize);
            }
            break;
            case TAG_Bitfield:
            case TAG_NameBitfield:
            {
                uint32_t v { 0 };
                Exchange_I32(v);
                visitor->Visit_Bitfield(v);
            }
            break;
            case TAG_SkipMark:
                visitor->Visit_SkipMark();
            break;
            case TAG_StringOrArray_E:
            case TAG_StringOrArray_8E:
            {
                if ((GetHeaderBFlags() >> 2) & 1) {
                    std::string s {};
                    Exchange_String(s);
                    visitor->Visit_StringOrArray(s);
                }
                else
                {
                    uint32_t v { 0 };
                    Exchange_I32(v);
                    visitor->Visit_StringOrArray(v);
                }
            }
            break;
            case TAG_StringArray:
            {
                if ((GetHeaderBFlags() >> 2) & 1)
                {
                    if ((GetHeaderBFlags() >> 3) & 1)
                    {
                        //Impl #1
                        uint32_t capacity { 0 };
                        Exchange_I32(capacity);
                        std::vector<std::string> arr;
                        if (capacity) { arr.reserve(capacity); }

                        for (int i = 0; i < capacity; i++)
                        {
                            std::string s;
                            Exchange_String(s);
                            arr.push_back(s);
                        }

                        visitor->Visit_StringArray(arr);
                    }
                    else
                    {
                        //Impl #2 (at 0x458CD0)
                        spdlog::error("[OP| +{:X}] TAG_StringArray (impl 2, opcode {:2X}). Not implemented", offset, byte);
                        m_contextFlags |= ContextFlag::CF_ERROR;
                        assert(false);
                    }
                }
                else
                {
                    uint32_t v { 0 };
                    Exchange_I32(v);
                    visitor->Visit_I32(v);
                }
            }
            break;
            case TAG_UNKNOWN:
            case NO_TAG:
            default:
            {
                visitor->Visit_UnknownTag(byte);
                m_contextFlags |= ContextFlag::CF_ERROR;
                spdlog::error("[OP| +{:X}] Unknown opcode {:2X}", offset, byte);
                assert(false);
            }
            break;
        }
    }

    uint8_t PRPWalker::ExchangeHeader(unsigned int /*type*/) {
        return m_walker.ReadUInt8();
    }

    void PRPWalker::ExchangeFooter(unsigned int type) {
        if (type == EPropertyType::Type_14) {
            if (GetHeaderBFlags() & 1) {
                m_walker.ReadUInt8();
            }
        }
    }

    void PRPWalker::ExchangeContainer(unsigned int* capacity) {
        ExchangeHeader(EPropertyType::Type_10);

        const uint32_t res = m_walker.ReadUInt32();
        if (capacity) {
            *capacity = res;
        }
    }

    void PRPWalker::BeginArray(unsigned int size) {
        if (!(GetHeaderBFlags() & 1))
            return;

        m_walker.ReadUInt8();

        if (GetHeaderBFlags() & 1)
        {
            m_walker.ReadUInt32();
            //TODO: Maybe we should check size constrains here? (walker->m_walker.ReadUInt32() == size)
        }
    }

    void PRPWalker::EndArray() {
        m_walker.ReadUInt8();
    }

    void PRPWalker::Exchange_I8(uint8_t& result) { result = GetCurrentByte(); }

    void PRPWalker::Exchange_I16(uint16_t& result) { result = m_walker.ReadUInt16(); }

    void PRPWalker::Exchange_I32(uint32_t& result) { result = m_walker.ReadUInt32(); }

    void PRPWalker::Exchange_F32(float& result) { result = m_walker.Read<float>(); }

    void PRPWalker::Exchange_F64(double& result) { result = m_walker.Read<double>(); }

    void PRPWalker::Exchange_String(std::string& result) {
        if (((GetHeaderBFlags() >> 3) & 1) == 0) {
            uint32_t length { 0 };
            Exchange_I32(length);

            assert(length > 0);

            result.resize(length + 1);
            result[length] = 0x0;

            Exchange_RawContents(result.data(), length);
        }
        else
        {
            const uint32_t index = m_walker.ReadUInt32();
            if (index >= m_tokenTable.size()) {
                spdlog::error("IPRPVisitor::Visit_String| Index of string is out of bounds! Index is {:8X}", index);
                assert(false);
                return;
            }

            result = m_tokenTable[index];
        }
    }

    void PRPWalker::Exchange_RawBuffer(std::shared_ptr<uint8_t[]>& buffer, size_t& bufferSize) {
        uint32_t size { 0 };

        Exchange_I32(size);
        if (size) {
            bufferSize = size;

            buffer = std::make_shared<uint8_t[]>(size);
            m_walker.ReadArray(buffer.get(), bufferSize);
        }
    }

    void PRPWalker::Exchange_RawContents(char* buffer, size_t bufferSize) {
        assert(bufferSize > 0);
        m_walker.ReadArray(buffer, bufferSize);
    }

    template<>
    void PRPWalker::ExchangeArray<uint32_t>(uint32_t* data, unsigned int size)
    {
        BeginArray(size);

        auto sz = size;
        uint32_t* pCurrent = data;
        do {
            ExchangeHeader(EPropertyType::Type_7);
            Exchange_I32(reinterpret_cast<uint32_t&>(*pCurrent));
            ExchangeFooter(EPropertyType::Type_7);

            ++pCurrent;
            --sz;
        } while (sz);

        EndArray();
    }

    template<>
    void PRPWalker::ExchangeArray<float>(float* data, unsigned int size)
    {
        BeginArray(size);

        auto sz = size;
        float* pCurrent = data;
        do {
            ExchangeHeader(EPropertyType::Type_7);
            Exchange_F32(reinterpret_cast<float&>(*pCurrent));
            ExchangeFooter(EPropertyType::Type_7);

            ++pCurrent;
            --sz;
        } while (sz);

        EndArray();
    }

    uint8_t PRPWalker::GetCurrentByte() const {
        return m_walker.ReadInt8();
    }

    size_t PRPWalker::GetCurrentOffset() const {
        return m_walker.GetPosition();
    }
}