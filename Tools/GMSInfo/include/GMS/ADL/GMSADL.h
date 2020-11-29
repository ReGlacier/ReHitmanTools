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
            value.Unknown4  = binaryWalker.Read<uint32_t>();
            value.Unknown8  = binaryWalker.Read<uint32_t>();
            value.UnknownC  = binaryWalker.Read<uint32_t>();
            value.DataPos   = binaryWalker.Read<uint32_t>();
            value.Unknown14  = binaryWalker.Read<uint32_t>();
            value.Unknown18  = binaryWalker.Read<uint32_t>();
            value.Unknown1C  = binaryWalker.Read<uint32_t>();
            value.Unknown20  = binaryWalker.Read<uint32_t>();
            value.Unknown24  = binaryWalker.Read<uint32_t>();
            value.Unknown28 = binaryWalker.Read<uint32_t>();
            value.Unknown2C = binaryWalker.Read<uint32_t>();
            value.Unknown30 = binaryWalker.Read<uint32_t>();
            value.Unknown34 = binaryWalker.Read<uint32_t>();
            value.Unknown38 = binaryWalker.Read<uint32_t>();
            value.BaseGeomsCount = binaryWalker.Read<uint32_t>();
        }

        static void Write(BinaryWalker& binaryWalker, const SGMSUncompressedHeader& value)
        {
            binaryWalker.Write<uint32_t>(value.TotalEntitiesCountPos);
            binaryWalker.Write<uint32_t>(value.Unknown4);
            binaryWalker.Write<uint32_t>(value.Unknown8);
            binaryWalker.Write<uint32_t>(value.UnknownC);
            binaryWalker.Write<uint32_t>(value.DataPos);
            binaryWalker.Write<uint32_t>(value.Unknown14);
            binaryWalker.Write<uint32_t>(value.Unknown18);
            binaryWalker.Write<uint32_t>(value.Unknown1C);
            binaryWalker.Write<uint32_t>(value.Unknown20);
            binaryWalker.Write<uint32_t>(value.Unknown24);
            binaryWalker.Write<uint32_t>(value.Unknown28);
            binaryWalker.Write<uint32_t>(value.Unknown2C);
            binaryWalker.Write<uint32_t>(value.Unknown30);
            binaryWalker.Write<uint32_t>(value.Unknown34);
            binaryWalker.Write<uint32_t>(value.Unknown38);
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

            value.PrimitiveBufGroupNameOffset = binaryWalker.Read<uint32_t>();
            value.Unknown4 = binaryWalker.Read<uint32_t>();
            value.Unknown8 = binaryWalker.Read<uint32_t>();
            value.UnknownC = binaryWalker.Read<uint32_t>();
            value.Unknown10 = binaryWalker.Read<uint32_t>();
            value.TypeId = static_cast<Glacier::TypeId>(binaryWalker.Read<uint32_t>());
            value.Unknown18 = binaryWalker.Read<uint32_t>();
            value.Unknown1C = binaryWalker.Read<uint32_t>();
            value.PRMOffset = binaryWalker.Read<uint32_t>();
            value.Unknown24 = binaryWalker.Read<uint32_t>();
            value.Unknown28 = binaryWalker.Read<uint32_t>();
            value.Unknown2C = binaryWalker.Read<uint32_t>();
            value.Unknown30 = binaryWalker.Read<uint32_t>();
            value.Unknown34 = binaryWalker.Read<uint32_t>();
            value.Unknown38 = binaryWalker.Read<uint32_t>();
            value.Unknown3C = binaryWalker.Read<uint32_t>();
        }

        static void Write(BinaryWalker& binaryWalker, const SGMSBaseGeom& value)
        {
            throw std::exception { "NOT IMPLEMENTED" };
//            binaryWalker.Write<uint32_t>(value.PrimitiveId);
//            binaryWalker.Write<uint32_t>(value.Unknown2);
//            binaryWalker.Write<uint32_t>(value.Unknown3);
//            binaryWalker.Write<uint32_t>(value.PrimitiveOffset);
//            binaryWalker.Write<uint32_t>(value.Unknown5);
//            binaryWalker.Write<uint32_t>(static_cast<uint32_t>(value.TypeId));
//            binaryWalker.Write<uint32_t>(value.Unknown7);
//            binaryWalker.Write<uint32_t>(value.Unknown8);
        }
    };
}