#pragma once

#include <PRP/PRP_ETag_Helpers.h>
#include <PRP/PRPWalker.h>

namespace ReGlacier
{
    class IPRPVisitor
    {
    public:
        virtual ~IPRPVisitor() = default;

        virtual void Visit_BeginArray(uint32_t capacity) = 0;
        virtual void Visit_EndArray() = 0;
        virtual void Visit_BeginObject() = 0;
        virtual void Visit_EndObject() = 0;
        virtual void Visit_Container(uint32_t) = 0;
        virtual void Visit_Bitfield(uint32_t) = 0;
        virtual void Visit_Bool(bool value) = 0;
        virtual void Visit_Char(char ch) = 0;
        virtual void Visit_I8(uint8_t result) = 0;
        virtual void Visit_I16(uint16_t result) = 0;
        virtual void Visit_I32(uint32_t result) = 0;
        virtual void Visit_F32(float result) = 0;
        virtual void Visit_F64(double result) = 0;
        virtual void Visit_String(const std::string& result) = 0;
        virtual void Visit_RawBuffer(const std::shared_ptr<uint8_t[]>& buffer, size_t& bufferSize) = 0;
        virtual void Visit_StringArray(const std::vector<std::string>& stringArray) = 0;
        virtual void Visit_StringOrArray(const std::string& s) = 0;
        virtual void Visit_StringOrArray(uint32_t v) = 0;
        virtual void Visit_SkipMark() = 0;
        virtual void Visit_EndOfStream() = 0;
        virtual void Visit_UnknownTag(uint8_t byte) = 0;
    };
}