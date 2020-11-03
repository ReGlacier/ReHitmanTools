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
            char ch;

            while ((ch = binaryWalker.Read<char>()) != 0x0)
            {
                value.push_back(ch);
            }
        }

        static void Write(BinaryWalker& binaryWalker, const std::string& value)
        {
            binaryWalker.WriteArray<char>(value.data(), value.length());
        }
    };
}