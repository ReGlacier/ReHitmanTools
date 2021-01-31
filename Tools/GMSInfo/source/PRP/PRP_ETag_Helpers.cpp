#include <PRP/PRP_ETag_Helpers.h>

namespace ReGlacier
{
    static constexpr const char* kNoTag = "(NOT A TAG)";

    PRP_ETag PRP_ETag_Helpers::FromByte(uint8_t byte)
    {
        if (byte == 0x0E) return PRP_ETag::TAG_StringOrArray_E;
        if (byte == 0x8E) return PRP_ETag::TAG_StringOrArray_8E;
        if (byte == 0x0F) return PRP_ETag::TAG_StringArray;

        if (byte < PRP_ETag::TAG_Array || byte > PRP_ETag::TAG_NameBitfield)
        {
            return PRP_ETag::NO_TAG;
        }

        if (byte == 128 || (byte >= 16 && byte <= 123))
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
        RETAG(PRP_ETag::TAG_StringArray);
        RETAG(PRP_ETag::TAG_StringOrArray_E);
        RETAG(PRP_ETag::TAG_StringOrArray_8E);
        return kNoTag;
#undef RETAG
    }
}