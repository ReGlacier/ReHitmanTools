#pragma once

#include <BinaryWalker.h>
#include <BinaryWalkerADL.h>

#include <TEX/TEXTypes.h>

namespace ReGlacier
{
    template <>
    struct BinaryWalkerADL<STEXHeader>
    {
        static void Read(const BinaryWalker& binaryWalker, STEXHeader& header)
        {
            header.Table1Location = binaryWalker.Read<uint32_t>();
            header.Table2Location = binaryWalker.Read<uint32_t>();
            header.RawBufferLocation = binaryWalker.Read<uint32_t>();
            header.Unknown1 = binaryWalker.Read<uint32_t>();
        }

        static void Write(BinaryWalker& binaryWalker, const STEXHeader& header)
        {
            binaryWalker.Write<uint32_t>(header.Table1Location);
            binaryWalker.Write<uint32_t>(header.Table2Location);
            binaryWalker.Write<uint32_t>(header.RawBufferLocation);
            binaryWalker.Write<uint32_t>(header.Unknown1);
        }
    };

    template <>
    struct BinaryWalkerADL<ETEXEntityType>
    {
        static void Read(const BinaryWalker& binaryWalker, ETEXEntityType& type)
        {
            char buff[4];
            binaryWalker.ReadArray<char, 4>(&buff[0]);
            type = *(ETEXEntityType*)(&buff[0]);
        }

        static void Write(BinaryWalker& binaryWalker, const ETEXEntityType& entry)
        {
            binaryWalker.WriteArray<char, 4>((char*)&entry);
        }
    };

    template <>
    struct BinaryWalkerADL<STEXEntry>
    {
        static void Read(const BinaryWalker& binaryWalker, STEXEntry& entry)
        {
            entry.FileSize = binaryWalker.Read<uint32_t>();
            BinaryWalkerADL<ETEXEntityType>::Read(binaryWalker, entry.Type);
            BinaryWalkerADL<ETEXEntityType>::Read(binaryWalker, entry.Type2);
            entry.Index = binaryWalker.Read<int32_t>();
            entry.Height = binaryWalker.Read<int16_t>();
            entry.Width = binaryWalker.Read<int16_t>();
            entry.MipMapLevels = binaryWalker.Read<int32_t>();
            entry.Unknown1 = binaryWalker.Read<int32_t>();
            entry.Unknown2 = binaryWalker.Read<float>();
            entry.Unknown3 = binaryWalker.Read<int32_t>();
            BinaryWalkerADL<std::string>::Read(binaryWalker, entry.FileName);
        }

        static void Write(BinaryWalker& binaryWalker, const STEXEntry& entry)
        {
            binaryWalker.Write<uint32_t>(entry.FileSize);
            BinaryWalkerADL<ETEXEntityType>::Write(binaryWalker, entry.Type);
            BinaryWalkerADL<ETEXEntityType>::Write(binaryWalker, entry.Type2);
            binaryWalker.Write<int32_t>(entry.Index);
            binaryWalker.Write<int16_t>(entry.Height);
            binaryWalker.Write<int16_t>(entry.Width);
            binaryWalker.Write<int32_t>(entry.MipMapLevels);
            binaryWalker.Write<int32_t>(entry.Unknown1);
            binaryWalker.Write<float>(entry.Unknown2);
            binaryWalker.Write<int32_t>(entry.Unknown3);
            BinaryWalkerADL<std::string>::Write(binaryWalker, entry.FileName);
        }
    };

    template <>
    struct BinaryWalkerADL<STEXEntityAllocationInfo>
    {
        static void Read(const BinaryWalker& binaryWalker, STEXEntityAllocationInfo& entry)
        {
            entry.MipMapLevelsSize = binaryWalker.Read<int32_t>();
            entry.DataOffsets = binaryWalker.GetPosition();
            entry.Data = std::make_unique<char[]>(entry.MipMapLevelsSize);
            binaryWalker.ReadArray<char>(entry.Data.get(), entry.MipMapLevelsSize);
        }

        static void Write(BinaryWalker& binaryWalker, const STEXEntityAllocationInfo& entry)
        {
            binaryWalker.Write<int32_t>(entry.MipMapLevelsSize);
            binaryWalker.Write<int32_t>(entry.DataOffsets);
            binaryWalker.WriteArray<char>(entry.Data.get(), entry.MipMapLevelsSize);
        }
    };
}