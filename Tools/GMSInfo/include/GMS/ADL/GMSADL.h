#pragma once

#include <GMS/GMSTypes.h>

#include <BinaryWalker.h>
#include <BinaryWalkerADL.h>

#include <GlacierTypeDefs.h>

namespace ReGlacier
{
    template <>
    struct BinaryWalkerADL<SGMSUncompressedHeader>
    {
        static void Read(const BinaryWalker& binaryWalker, SGMSUncompressedHeader& value)
        {
            binaryWalker.RequireSpace(sizeof(SGMSUncompressedHeader));

            value.TotalEntitiesCountPos = binaryWalker.Read<uint32_t>();
            value.Unknown1  = binaryWalker.Read<uint32_t>();
            value.Unknown2  = binaryWalker.Read<uint32_t>();
            value.Unknown3  = binaryWalker.Read<uint32_t>();
            value.DataPos   = binaryWalker.Read<uint32_t>();
            value.Unknown5  = binaryWalker.Read<uint32_t>();
            value.Unknown6  = binaryWalker.Read<uint32_t>();
            value.Unknown7  = binaryWalker.Read<uint32_t>();
            value.Unknown8  = binaryWalker.Read<uint32_t>();
            value.Unknown9  = binaryWalker.Read<uint32_t>();
            value.Unknown10 = binaryWalker.Read<uint32_t>();
            value.Unknown11 = binaryWalker.Read<uint32_t>();
            value.Unknown12 = binaryWalker.Read<uint32_t>();
            value.Unknown13 = binaryWalker.Read<uint32_t>();
            value.Unknown14 = binaryWalker.Read<uint32_t>();
            value.BaseGeomsCount = binaryWalker.Read<uint32_t>();
        }

        static void Write(BinaryWalker& binaryWalker, const SGMSUncompressedHeader& value)
        {
            binaryWalker.Write<uint32_t>(value.TotalEntitiesCountPos);
            binaryWalker.Write<uint32_t>(value.Unknown1);
            binaryWalker.Write<uint32_t>(value.Unknown2);
            binaryWalker.Write<uint32_t>(value.Unknown3);
            binaryWalker.Write<uint32_t>(value.DataPos);
            binaryWalker.Write<uint32_t>(value.Unknown5);
            binaryWalker.Write<uint32_t>(value.Unknown6);
            binaryWalker.Write<uint32_t>(value.Unknown7);
            binaryWalker.Write<uint32_t>(value.Unknown8);
            binaryWalker.Write<uint32_t>(value.Unknown9);
            binaryWalker.Write<uint32_t>(value.Unknown10);
            binaryWalker.Write<uint32_t>(value.Unknown11);
            binaryWalker.Write<uint32_t>(value.Unknown12);
            binaryWalker.Write<uint32_t>(value.Unknown13);
            binaryWalker.Write<uint32_t>(value.Unknown14);
            binaryWalker.Write<uint32_t>(value.BaseGeomsCount);
        }
    };

    template <>
    struct BinaryWalkerADL<SGMSEntry>
    {
        static void Read(const BinaryWalker& binaryWalker, SGMSEntry& value)
        {
            binaryWalker.RequireSpace(sizeof(SGMSEntry));

            value.Unknown1 = binaryWalker.Read<uint32_t>();
            value.TypeInfoPos = binaryWalker.Read<uint32_t>();
        }

        static void Write(BinaryWalker& binaryWalker, const SGMSEntry& value)
        {
            binaryWalker.Write<uint32_t>(value.Unknown1);
            binaryWalker.Write<uint32_t>(value.TypeInfoPos);
        }
    };

    template <>
    struct BinaryWalkerADL<SGMSBaseGeom>
    {
        static void Read(const BinaryWalker& binaryWalker, SGMSBaseGeom& value)
        {
            binaryWalker.RequireSpace(sizeof(SGMSBaseGeom));

            value.PrimitiveId = binaryWalker.Read<uint32_t>();
            value.Unknown2 = binaryWalker.Read<uint32_t>();
            value.Unknown3 = binaryWalker.Read<uint32_t>();
            value.PrimitiveOffset = binaryWalker.Read<uint32_t>();
            value.Unknown5 = binaryWalker.Read<uint32_t>();
            value.TypeId = static_cast<Glacier::TypeId>(binaryWalker.Read<uint32_t>());
            value.Unknown7 = binaryWalker.Read<uint32_t>();
            value.Unknown8 = binaryWalker.Read<uint32_t>();
        }

        static void Write(BinaryWalker& binaryWalker, const SGMSBaseGeom& value)
        {
            binaryWalker.Write<uint32_t>(value.PrimitiveId);
            binaryWalker.Write<uint32_t>(value.Unknown2);
            binaryWalker.Write<uint32_t>(value.Unknown3);
            binaryWalker.Write<uint32_t>(value.PrimitiveOffset);
            binaryWalker.Write<uint32_t>(value.Unknown5);
            binaryWalker.Write<uint32_t>(static_cast<uint32_t>(value.TypeId));
            binaryWalker.Write<uint32_t>(value.Unknown7);
            binaryWalker.Write<uint32_t>(value.Unknown8);
        }
    };
}