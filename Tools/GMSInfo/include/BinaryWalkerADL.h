#pragma once

#include <BinaryWalker.h>

#include <string>
#include <array>

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

    template <typename T, size_t S>
    struct BinaryWalkerADL<std::array<T, S>>
    {
        static void Read(const BinaryWalker& binaryWalker, std::array<T, S>& array)
        {
            if constexpr (std::is_trivial_v<T>)
            {
                binaryWalker.ReadArray(array);
            }
            else
            {
                for (auto& item : array)
                {
                    BinaryWalkerADL<T>::Read(binaryWalker, item);
                }
            }
        }

        static void Write(BinaryWalker& binaryWalker, const std::array<T, S>& array)
        {
            if constexpr (std::is_trivial_v<T>)
            {
                binaryWalker.WriteArray(array);
            }
            else
            {
                for (const auto& item : array)
                {
                    BinaryWalkerADL<T>::Write(binaryWalker, item);
                }
            }
        }
    };
}