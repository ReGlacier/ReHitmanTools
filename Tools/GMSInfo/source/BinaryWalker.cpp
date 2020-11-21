#include <BinaryWalker.h>

namespace ReGlacier
{
    IBaseStreamWalker::IBaseStreamWalker(size_t size, size_t offset)
        : m_size(size)
        , m_offset(offset)
    {}

    IBaseStreamWalker::IBaseStreamWalker(const IBaseStreamWalker& copy)
    {
        IBaseStreamWalker::operator=(copy);
    }

    IBaseStreamWalker::IBaseStreamWalker(IBaseStreamWalker&& move) noexcept
    {
        IBaseStreamWalker::operator=(std::move(move));
    }

    IBaseStreamWalker& IBaseStreamWalker::operator=(const IBaseStreamWalker& copy)
    {
        if (&copy != this)
        {
            m_size = copy.m_size;
            m_offset = copy.m_offset;
        }
        return *this;
    }

    IBaseStreamWalker& IBaseStreamWalker::operator=(IBaseStreamWalker&& move) noexcept
    {
        if (&move != this)
        {
            m_size = move.m_size; move.m_size = 0;
            m_offset = move.m_offset; move.m_offset = 0;
        }

        return *this;
    }

    void IBaseStreamWalker::Reset()
    {
        m_offset = 0;
    }

    void IBaseStreamWalker::Seek(size_t offset, SeekType seekType)
    {
        switch (seekType)
        {
            case SeekType::FROM_CURRENT_POSITION:
            {
                if (m_offset + offset >= m_size)
                    throw std::out_of_range { "Seek operation failed. Current offset is greater that buffer size" };

                m_offset += offset;
            }
                break;
            case SeekType::FROM_BEGIN:
            {
                if (offset >= m_size)
                    throw std::out_of_range { "Seek operation failed. Current offset is greater that buffer size" };

                m_offset = offset;
            }
                break;
            case SeekType::FROM_END:
            {
                if (offset >= m_size)
                    throw std::out_of_range { "Seek operation failed. Current offset is greater that buffer size" };

                m_offset = m_size - offset;
            }
                break;
        }
    }

    void IBaseStreamWalker::Align(size_t align, char padByte)
    {
        auto position = GetPosition();

        if (position % align == 0)
            return;

        auto offset = align - position % align;

        for (auto i = 0; i < offset; i++)
        {
            WriteInt8(padByte);
        }
    }

    size_t IBaseStreamWalker::GetPosition() const
    {
        return m_offset;
    }

    void IBaseStreamWalker::RequireSpace(size_t space) const
    {
        if (m_offset + space > m_size)
            throw std::runtime_error { fmt::format("Not enough space! Required {} available {} (offset {:X})", space, m_size - m_offset, m_offset) };
    }

    // -------------------------------------------------------------------------------

    BinaryWalker::BinaryWalker(uint8_t* buffer, size_t size)
        : IBaseStreamWalker(size, 0)
        , m_buffer(buffer)
    {}

    BinaryWalker::BinaryWalker(const BinaryWalker& copy) noexcept
        : IBaseStreamWalker(copy)
    {
        operator=(copy);
    }

    BinaryWalker::BinaryWalker(BinaryWalker&& move) noexcept
        : IBaseStreamWalker(std::move(move))
    {
        operator=(std::move(move));
    }

    BinaryWalker& BinaryWalker::operator=(const BinaryWalker& copy) noexcept
    {
        if (&copy != this)
        {
            IBaseStreamWalker::operator=(copy);
            m_buffer = copy.m_buffer;
        }

        return *this;
    }

    BinaryWalker& BinaryWalker::operator=(BinaryWalker&& move) noexcept
    {
        if (&move != this)
        {
            IBaseStreamWalker::operator=(std::move(move));
            m_buffer = move.m_buffer; move.m_buffer = nullptr;
        }

        return *this;
    }

    BinaryWalker::operator bool() const { return m_buffer != nullptr && m_size > 0; }

    void BinaryWalker::WriteUInt8(uint8_t value)
    {
        RequireSpace(sizeof(uint8_t));

        char* ptr = (&((char*)m_buffer)[m_offset]);
        *(uint8_t*)(ptr) = value;

        m_offset += sizeof(uint8_t);
    }

    void BinaryWalker::WriteInt8(int8_t value)
    {
        RequireSpace(sizeof(int8_t));

        char* ptr = (&((char*)m_buffer)[m_offset]);
        *(int8_t*)(ptr) = value;

        m_offset += sizeof(int8_t);
    }

    void BinaryWalker::WriteUInt16(uint16_t value)
    {
        RequireSpace(sizeof(uint16_t));

        char* ptr = (&((char*)m_buffer)[m_offset]);
        *(uint16_t*)(ptr) = value;

        m_offset += sizeof(uint16_t);
    }

    void BinaryWalker::WriteInt16(int16_t value)
    {
        RequireSpace(sizeof(int16_t));

        char* ptr = (&((char*)m_buffer)[m_offset]);
        *(int16_t*)(ptr) = value;

        m_offset += sizeof(int16_t);
    }

    void BinaryWalker::WriteUInt32(uint32_t value)
    {
        RequireSpace(sizeof(uint32_t));

        char* ptr = (&((char*)m_buffer)[m_offset]);
        *(uint32_t*)(ptr) = value;

        m_offset += sizeof(uint32_t);
    }

    void BinaryWalker::WriteInt32(int32_t value)
    {
        RequireSpace(sizeof(int32_t));

        char* ptr = (&((char*)m_buffer)[m_offset]);
        *(int32_t*)(ptr) = value;

        m_offset += sizeof(int32_t);
    }

    uint8_t BinaryWalker::ReadUInt8() const
    {
        RequireSpace(sizeof(uint8_t));

        char* ptr = (&((char*)m_buffer)[m_offset]);

        m_offset += sizeof(uint8_t);
        return *((uint8_t*)ptr);
    }

    int8_t BinaryWalker::ReadInt8() const
    {
        RequireSpace(sizeof(int8_t));

        char* ptr = (&((char*)m_buffer)[m_offset]);

        m_offset += sizeof(int8_t);
        return *((int8_t*)ptr);
    }

    uint16_t BinaryWalker::ReadUInt16() const
    {
        RequireSpace(sizeof(uint16_t));

        char* ptr = (&((char*)m_buffer)[m_offset]);

        m_offset += sizeof(uint16_t);
        return *((uint16_t*)ptr);
    }

    int16_t BinaryWalker::ReadInt16() const
    {
        RequireSpace(sizeof(int16_t));

        char* ptr = (&((char*)m_buffer)[m_offset]);

        m_offset += sizeof(int16_t);
        return *((int16_t*)ptr);
    }

    uint32_t BinaryWalker::ReadUInt32() const
    {
        RequireSpace(sizeof(uint32_t));

        char* ptr = (&((char*)m_buffer)[m_offset]);

        m_offset += sizeof(uint32_t);
        return *((uint32_t*)ptr);
    }

    int32_t BinaryWalker::ReadInt32() const
    {
        RequireSpace(sizeof(int32_t));

        char* ptr = (&((char*)m_buffer)[m_offset]);

        m_offset += sizeof(int32_t);
        return *((int32_t*)ptr);
    }

    std::string BinaryWalker::ReadZString(int limit) const
    {
        std::string str;

        uint8_t ch = 0;
        size_t it = 0;

        do {
            ch = ReadUInt8();

            if (ch != 0)
            {
                str.push_back(ch);
            }

            ++it;
        } while (ch != 0 && it < limit);

        return str;
    }
}