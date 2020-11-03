#include <BinaryWalker.h>

namespace ReGlacier
{
    BinaryWalker::BinaryWalker(std::unique_ptr<uint8_t[]>&& buffer, size_t size)
        : m_buffer(std::move(buffer))
        , m_size(size)
    {}

    void BinaryWalker::Reset() { m_offset = 0; }

    void BinaryWalker::Seek(size_t offset, SeekType seekType)
    {
        if (m_offset >= m_size)
            throw std::out_of_range { "Seek operation failed. Current offset is greater that buffer size" };

        switch (seekType)
        {
            case SeekType::FROM_CURRENT_POSITION: m_offset += offset; break;
            case SeekType::FROM_BEGIN: m_offset = offset; break;
            case SeekType::FROM_END: m_offset = m_size - offset; break;
        }
    }

    std::unique_ptr<uint8_t []>&& BinaryWalker::Take()
    {
        m_offset = 0;
        m_size   = 0;
        return std::move(m_buffer);
    }

    void BinaryWalker::Align(size_t align, char padByte)
    {
        auto position = GetPosition();

        if (position % align == 0)
            return;

        auto offset = align - position % align;

        for (auto i = 0; i < offset; i++)
        {
            Write<char>(padByte);
        }
    }

    size_t BinaryWalker::GetPosition() const
    {
        return m_offset;
    }

    std::intptr_t BinaryWalker::At() const
    {
        return (std::intptr_t)(m_buffer.get() + m_offset);
    }
}