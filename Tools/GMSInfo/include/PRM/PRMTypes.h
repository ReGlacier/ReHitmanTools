#pragma once

/**
 * @credits: John "Cryect" Rittenhouse - Hitman PRM to Obj converter 0.1 (8/12/2006)
 */

#include <cstdint>

namespace ReGlacier
{
    struct PRMHeader
    {
        uint32_t ChunkPos;
        uint32_t ChunkNum;
        uint32_t ChunkPos2;
        uint32_t Zero;
    };

    struct PRMChunk
    {
        int32_t Pos;
        int32_t Size;
        int32_t IsGeometry;
        int32_t Unknown2;
    };
}