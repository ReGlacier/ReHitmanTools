#pragma once

#include <cstdint>
#include <vector>
#include <string>
#include <string_view>

namespace BM::LOC::Internal
{
    struct CompilerUtils
    {
        static size_t GetStringAlignedLength(std::string_view str, int alignOut = 4);
        static size_t WriteZStringAligned(std::vector<uint8_t>& buffer, size_t offset, std::string_view str);
        static size_t WriteZString(std::vector<uint8_t>& buffer, size_t offset, const std::string& str);
        static void WriteByte(std::vector<uint8_t>& buffer, size_t offset, uint8_t b);
        static void WriteUInt32(std::vector<uint8_t>& buffer, size_t offset, uint32_t v);
    };
}