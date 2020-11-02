#pragma once

#include <BinaryWalker.h>

#include <string>

namespace ReGlacier
{
    template <typename T>
    struct BinaryWalkerADL
    {
        static void Read(const BinaryWalker& binaryWalker, T& value) {}
        static void Write(BinaryWalker& binaryWalker, const T& value) {}
    };

    template <>
    struct BinaryWalkerADL<std::string>
    {
        static constexpr char kEOS = 0x0;

        static void Read(const BinaryWalker& binaryWalker, std::string& value)
        {
            char ch = binaryWalker.Read<char>();

            while (ch != kEOS)
            {
                value.push_back(ch);
                ch = binaryWalker.Read<char>();
            }
        }

        static void Write(BinaryWalker& binaryWalker, const std::string& value)
        {
            for (const auto& ch : value)
            {
                binaryWalker.Write<char>(ch);
            }
            binaryWalker.Write<char>(kEOS);
        }
    };
}