#pragma once

#include <PRP/PRPTypes.h>

#include <BinaryWalker.h>
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
}