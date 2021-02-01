#pragma once

#include <PRP/IPRPVisitor.h>

namespace ReGlacier
{
    class DebugPRPVisitor final : public IPRPVisitor
    {
    public:
        void Visit_BeginArray(uint32_t capacity) override;
        void Visit_EndArray() override;
        void Visit_BeginObject() override;
        void Visit_EndObject() override;
        void Visit_Container(uint32_t) override;
        void Visit_Bitfield(uint32_t) override;
        void Visit_Bool(bool value) override;
        void Visit_Char(char ch) override;
        void Visit_I8(uint8_t result) override;
        void Visit_I16(uint16_t result) override;
        void Visit_I32(uint32_t result) override;
        void Visit_F32(float result) override;
        void Visit_F64(double result) override;
        void Visit_String(const std::string& result) override;
        void Visit_RawBuffer(const std::shared_ptr<uint8_t[]>& buffer, size_t& bufferSize) override;
        void Visit_StringArray(const std::vector<std::string>& stringArray) override;
        void Visit_StringOrArray(const std::string& s) override;
        void Visit_StringOrArray(uint32_t v) override;
        void Visit_SkipMark() override;
        void Visit_EndOfStream() override;
        void Visit_UnknownTag(uint8_t byte) override;
    };
}