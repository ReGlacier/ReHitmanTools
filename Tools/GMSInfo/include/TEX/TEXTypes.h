#pragma once

#include <cstdint>

namespace ReGlacier
{
    struct STEXHeader
    {
        uint32_t Table1Location;
        uint32_t Table2Location;
        uint32_t RawBufferLocation;
        uint32_t Unknown1;
    };

    enum ETEXEntityType : unsigned int
    {
        BITMAP_PAL         = 'PALN',
        BITMAP_PAL_OPAC    = 'PALO',
        BITMAP_32          = 'RGBA',
        BITMAP_U8V8        = 'U8V8',
        BITMAP_DXT1        = 'DXT1',
        BITMAP_DXT3        = 'DXT3',
        BITMAP_I8          = '  I8'
    };

    struct STEXEntry
    {
        uint32_t FileSize;
        ETEXEntityType Type;
        ETEXEntityType Type2;
        int32_t Index;
        int16_t Height;
        int16_t Width;
        int32_t MipMapLevels;
        int32_t Unknown1;
        int32_t Unknown2;
        int32_t Unknown3;
        std::string FileName;
    };
}