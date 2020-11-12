#pragma once

#include <BinaryWalker.h>
#include <BinaryWalkerADL.h>

#include <PRM/PRMTypes.h>

namespace ReGlacier
{
    template <>
    struct BinaryWalkerADL<PRMHeader>
    {
        static void Read(const BinaryWalker& binaryWalker, PRMHeader& value)
        {
            value.ChunkPos  = binaryWalker.Read<uint32_t>();
            value.ChunkNum  = binaryWalker.Read<uint32_t>();
            value.ChunkPos2 = binaryWalker.Read<uint32_t>();
            value.Zero      = binaryWalker.Read<uint32_t>();
        }

        static void Write(BinaryWalker& binaryWalker, const PRMHeader& value)
        {
            binaryWalker.Write<uint32_t>(value.ChunkPos);
            binaryWalker.Write<uint32_t>(value.ChunkNum);
            binaryWalker.Write<uint32_t>(value.ChunkPos2);
            binaryWalker.Write<uint32_t>(value.Zero);
        }
    };

    template <>
    struct BinaryWalkerADL<PRMChunk>
    {
        static void Read(const BinaryWalker& binaryWalker, PRMChunk& value)
        {
            value.Pos = binaryWalker.Read<int32_t>();
            value.Size = binaryWalker.Read<int32_t>();
            value.IsGeometry = binaryWalker.Read<int32_t>();
            value.Unknown2 = binaryWalker.Read<int32_t>();
        }

        static void Write(BinaryWalker& binaryWalker, const PRMChunk& value)
        {
            binaryWalker.Write<int32_t>(value.Pos);
            binaryWalker.Write<int32_t>(value.Size);
            binaryWalker.Write<int32_t>(value.IsGeometry);
            binaryWalker.Write<int32_t>(value.Unknown2);
        }
    };
}