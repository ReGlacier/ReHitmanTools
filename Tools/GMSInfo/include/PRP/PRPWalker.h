/**
 * @file PRPVFSM
 *
 * PRP File format Visitor FSM
 */
#pragma once

#include <PRP/PRPTypes.h>

#include <BinaryWalker.h>

#include <string>
#include <memory>
#include <cstdint>
#include <variant>
#include <map>

namespace ReGlacier
{
    class IPRPVisitor;

    class PRPWalker
    {
        friend class IPRPVisitor;
    public:
        using ZString = std::string;
        using ZRefTab = std::vector<std::string>;
        using ZI32Vec = std::vector<uint32_t>;
        using ZF32Vec = std::vector<float>;

        using ZDefine_t = std::variant<
                bool,
                uint8_t,
                uint16_t,
                uint32_t,
                float,
                double,
                ZString,
                ZI32Vec,
                ZF32Vec,
                ZRefTab>;

    private:
        std::unique_ptr<uint8_t[]> m_buffer;
        size_t m_bufferSize { 0 };

        std::vector<std::string> m_tokenTable {};
        std::map<std::string, ZDefine_t> m_zDefines {};

        SPRP m_header { 0 };
        BinaryWalker m_walker {};
        uint32_t m_totalObjectsPresented { 0 };

        enum ContextFlag : unsigned int
        {
            CF_NONE      = 0,
            CF_EOS       = 1 << 0,
            CF_ERROR     = 1 << 1,
            CF_OBJECT    = 1 << 2,
            CF_ARRAY     = 1 << 3,
            CF_CONTAINER = 1 << 4
        };

        unsigned int m_contextFlags { CF_NONE };

    public:
        PRPWalker(std::unique_ptr<uint8_t[]>&& buffer, size_t bufferSize);

        [[nodiscard]] uint32_t GetHeaderBFlags() const { return m_header.bFlags; }
        [[nodiscard]] uint32_t GetTotalObjectsCount() const { return m_totalObjectsPresented; }

        bool Prepare(IPRPVisitor* visitor);

    private:
        bool ValidateHeader();
        bool LoadTokenTable();
        bool LoadZDefines(IPRPVisitor* visitor);
        bool LoadProperties(IPRPVisitor* visitor);
        void PrepareByte(IPRPVisitor* visitor, uint8_t byte);

        uint8_t ExchangeHeader(unsigned int type);
        void ExchangeFooter(unsigned int type);
        void ExchangeContainer(unsigned int* capacity);

        void BeginArray(unsigned int size);
        void EndArray();

        void Exchange_I8(uint8_t& result);
        void Exchange_I16(uint16_t& result);
        void Exchange_I32(uint32_t& result);
        void Exchange_F32(float& result);
        void Exchange_F64(double& result);
        void Exchange_String(std::string& result);
        void Exchange_RawBuffer(std::shared_ptr<uint8_t[]>& buffer, size_t& bufferSize);
        void Exchange_RawContents(char* buffer, size_t bufferSize);

        template <typename T> void ExchangeArray(T* data, unsigned int size);
        template <> void ExchangeArray<uint32_t>(uint32_t* data, unsigned int size);
        template <> void ExchangeArray<float>(float* data, unsigned int size);

    private:
        uint8_t GetCurrentByte() const;

    public:
        size_t GetCurrentOffset() const;
    };
}