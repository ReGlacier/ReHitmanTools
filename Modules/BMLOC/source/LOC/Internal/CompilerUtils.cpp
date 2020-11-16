#include <BM/LOC/Internal/CompilerUtils.h>
#include <stdexcept>

namespace BM::LOC::Internal
{
    size_t CompilerUtils::GetStringAlignedLength(std::string_view str, int alignOut)
    {
        return str.length() + 1 + alignOut; // +1 - zero terminator, +4 alignment
    }

    size_t CompilerUtils::WriteZStringAligned(std::vector<uint8_t>& buffer, size_t offset, std::string_view str)
    {
        if (offset > buffer.size())
        {
            throw std::out_of_range {
                    "WriteZStringAligned: Out of range! Offset " + std::to_string(offset) +
                    " not included in " + std::to_string(buffer.size() - 1) };
        }

        size_t strLen = GetStringAlignedLength(str);

        if (offset + strLen > buffer.size())
        {
            throw std::out_of_range {
                    "WriteZStringAligned: Out of range! String buffer at " + std::to_string(offset) +
                    " with length " + std::to_string(strLen) + " not included in " + std::to_string(buffer.size() - 1)
            };
        }

        std::memset(&buffer[offset], 0x0, strLen);
        std::memcpy(&buffer[offset], str.data(), str.length());

        return offset + strLen;
    }

    size_t CompilerUtils::WriteZString(std::vector<uint8_t>& buffer, size_t offset, const std::string& str)
    {
        if (offset >= buffer.size())
        {
            throw std::out_of_range {
                    "WriteZString: Out of range! Offset " + std::to_string(offset) +
                    " not included in buffer " + std::to_string(buffer.size()) };
        }

        if (offset + str.length() + 1 >= buffer.size())
        {
            throw std::out_of_range {
                    "WriteZString: Out of range! Offset " + std::to_string(offset) +
                    " with length " + std::to_string(str.length() + 1) +
                    " not included in " + std::to_string(buffer.size() - 1) };
            return 0;
        }

        std::memcpy(&buffer[offset], str.data(), str.length());

        return offset + str.length() + 1;
    }

    void CompilerUtils::WriteByte(std::vector<uint8_t>& buffer, size_t offset, uint8_t b)
    {
        if (offset >= buffer.size())
        {
            throw std::out_of_range {
                    "WriteByte: Out of range! Offset " + std::to_string(offset) +
                    " with required space 1 (at " + std::to_string(offset + 1) +
                    ") not included in " + std::to_string(buffer.size() - 1)
            };
        }

        buffer[offset] = b;
    }

    void CompilerUtils::WriteUInt32(std::vector<uint8_t>& buffer, size_t offset, uint32_t v)
    {
        if (offset >= buffer.size())
        {
            throw std::out_of_range {
                    "WriteUInt32: Out of range! Offset " + std::to_string(offset) +
                    " not included in " + std::to_string(buffer.size() - 1)
            };
        }

        if (offset + sizeof(uint32_t) >= buffer.size())
        {
            throw std::out_of_range {
                    "WriteUInt32: Out of range! Offset " + std::to_string(offset) +
                    " with required space 4 (at " + std::to_string(offset + sizeof(uint32_t)) +
                    ") not included in " + std::to_string(buffer.size() - 1)
            };
        }

        uint32_t bs = v;
        std::memcpy(&buffer[offset], &bs, sizeof(uint32_t));
    }
}