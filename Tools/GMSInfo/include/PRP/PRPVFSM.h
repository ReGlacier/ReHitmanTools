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
    using PRPToken = uint32_t;

    struct ZToken
    {
        static constexpr PRPToken Void    = 0x0FFFFFFFF;
        static constexpr PRPToken Unknown = 0x0FFFFFFFE;
        static constexpr PRPToken Joker   = 0x0FFFFFFFD;
    };

    enum EPropertyType : unsigned int
    {
        Type_14 = 0xE,
        Type_10 = 0xA,
        Type_9 = 0x9,
        Type_8 = 0x8,
        Type_7 = 0x7,
        Type_2 = 0x2,
        Type_1 = 0x1
    };

    class PRPWalker
    {
        friend class IPRPVisitor;
    public:
        enum Flags : unsigned int {
            NO_FLAG = 0x0,
            END_OF_STREAM = 1 << 1,
            ERROR_IN_STREAM = 1 << 2
        };

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
        unsigned int m_flags { Flags::NO_FLAG };
        size_t m_bufferSize { 0 };
        size_t m_keysCount { 0 };
        size_t m_valuesOffset { 0 };
        std::vector<std::string> m_tokenTable {};
        std::map<std::string, ZDefine_t> m_zDefines {};
        SPRP m_header { 0 };
        BinaryWalker m_walker {};

    public:
        PRPWalker(std::unique_ptr<uint8_t[]>&& buffer, size_t bufferSize);

        [[nodiscard]] bool IsEOS() const { return m_flags & Flags::END_OF_STREAM; };
        [[nodiscard]] bool HasErrors() const { return m_flags & Flags::ERROR_IN_STREAM; }
        [[nodiscard]] uint32_t GetHeaderBFlags() const { return m_header.bFlags; }

        void Prepare(IPRPVisitor* visitor);

    private:
        bool LoadZDefines(IPRPVisitor* visitor);
        bool LoadProperties(IPRPVisitor* visitor);

    private:
        uint8_t GetCurrentByte() const;

    public:
        size_t GetCurrentOffset() const;
    };

    class IPRPVisitor
    {
    public:
        virtual ~IPRPVisitor() = default;

        /**
         * @brief This is debug function
         * @note DEBUG ONLY
         */
        virtual void OnByte(PRPWalker* walker, uint8_t byte) {}

        virtual void ExchangeHeader(PRPWalker* walker, unsigned int type);
        virtual void ExchangeFooter(PRPWalker* walker, unsigned int type);
        virtual void ExchangeContainer(PRPWalker* walker, unsigned int* capacity);

        virtual void BeginArray(PRPWalker* walker, PRPToken token, unsigned int size);
        virtual void EndArray(PRPWalker* walker);

        virtual void Visit_I8(PRPWalker* walker, PRPToken token, uint8_t& result);
        virtual void Visit_I16(PRPWalker* walker, PRPToken token, uint16_t& result);
        virtual void Visit_I32(PRPWalker* walker, PRPToken token, uint32_t& result);
        virtual void Visit_F32(PRPWalker* walker, PRPToken token, float& result);
        virtual void Visit_F64(PRPWalker* walker, PRPToken token, double& result);
        virtual void Visit_String(PRPWalker* walker, PRPToken token, std::string& result);
        virtual void Visit_RawBuffer(PRPWalker* walker, PRPToken token, std::shared_ptr<uint8_t[]>& buffer, size_t& bufferSize);
        virtual void Visit_RawContents(PRPWalker* walker, PRPToken token, char* buffer, size_t bufferSize);

        virtual void Visit_EndOfStream(PRPWalker* walker);

        template <typename T> void ExchangeArray(PRPWalker* walker, PRPToken token, T* data, unsigned int size);
        template <> void ExchangeArray<uint32_t>(PRPWalker* walker, PRPToken token, uint32_t* data, unsigned int size);
        template <> void ExchangeArray<float>(PRPWalker* walker, PRPToken token, float* data, unsigned int size);
    };
}