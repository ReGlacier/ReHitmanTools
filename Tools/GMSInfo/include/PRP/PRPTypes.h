#pragma once

#include <cstdint>
#include <string_view>

namespace ReGlacier
{
    enum PRP_ETag : uint8_t
    {
        TAG_Array               = 0x1,
        TAG_BeginObject         = 0x2,
        TAG_Reference           = 0x3,
        TAG_Container           = 0x4,
        TAG_Char                = 0x5,
        TAG_Bool                = 0x6,
        TAG_Int8                = 0x7,
        TAG_Int16               = 0x8,
        TAG_Int32               = 0x9,
        TAG_Float32             = 0xA,
        TAG_Float64             = 0xB,
        TAG_String              = 0xC,
        TAG_RawData             = 0xD,
        TAG_Bitfield            = 0x10,
        TAG_EndArray            = 0x7C,
        TAG_SkipMark            = 0x7D,
        TAG_EndObject           = 0x7E,
        TAG_EndOfStream         = 0x7F,
        TAG_NamedArray          = 0x81,
        TAG_BeginNamedObject    = 0x82,
        TAG_NamedReference      = 0x83,
        TAG_NamedContainer      = 0x84,
        TAG_NamedChar           = 0x85,
        TAG_NamedBool           = 0x86,
        TAG_NamedInt8           = 0x87,
        TAG_NamedInt16          = 0x88,
        TAG_NamedInt32          = 0x89,
        TAG_NamedFloat32        = 0x8A,
        TAG_NamedFloat64        = 0x8B,
        TAG_NamedString         = 0x8C,
        TAG_NamedRawData        = 0x8D,
        TAG_NameBitfield        = 0x8F,
        TAG_UNKNOWN, //14,16-123,128,142
        NO_TAG
    };

    struct PRP_ETag_Helpers
    {
        static PRP_ETag FromByte(uint8_t byte);
        static std::string_view ToString(PRP_ETag tag);
        static bool IsTagIncreaseDepthLevel(PRP_ETag tag);
        static bool IsTagDecreateDepthLevel(PRP_ETag tag);
    };
}