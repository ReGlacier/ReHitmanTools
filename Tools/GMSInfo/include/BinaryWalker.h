#pragma once

#include <cstdlib>
#include <cstdint>

#include <array>
#include <memory>
#include <stdexcept>
#include <type_traits>

#include <spdlog/spdlog.h>

namespace ReGlacier
{
    template <typename T>
    concept STLContainer = requires (T t) {
        t.size();
        t.begin();
        t.end();
        t.data();
        typename T::value_type;
    };

    class IBaseStreamWalker
    {
    protected:
        size_t m_size { 0 };
        mutable size_t m_offset { 0 };

    public:
        virtual ~IBaseStreamWalker() noexcept = default;
        IBaseStreamWalker() = default;
        IBaseStreamWalker(size_t size, size_t offset);

        IBaseStreamWalker(const IBaseStreamWalker& copy);
        IBaseStreamWalker(IBaseStreamWalker&& move) noexcept;
        IBaseStreamWalker& operator=(const IBaseStreamWalker& copy);
        IBaseStreamWalker& operator=(IBaseStreamWalker&& move) noexcept;

        enum class SeekType : uint8_t
        {
            FROM_CURRENT_POSITION, ///< Append offset to current offset
            FROM_BEGIN, ///< Override current offset to new value
            FROM_END ///< Override current offset to m_size - offset
        };

        static constexpr SeekType BEGIN = SeekType::FROM_BEGIN;
        static constexpr SeekType END   = SeekType::FROM_END;
        static constexpr SeekType CURR  = SeekType::FROM_CURRENT_POSITION;

        /**
         * Move internal offset to zero, equal to Seek(0, SeekType::FROM_BEGIN)
         */
        void Reset();

        /**
         * Move internal offset to passed value with passed rule
         * @param offset - new offset
         * @param seekType - new offset type
         */
        void Seek(size_t offset, SeekType seekType);

        /**
         * Write align bytes until current offset not aligned
         * @param align
         * @param padByte
         */
        void Align(size_t align, char padByte);

        /**
         * @return current offset
         */
        size_t GetPosition() const;

        /**
         * @brief Function for control of buffer space requirement for ADL parsers
         * @param space how much bytes required for data structure
         * @note this function will throw std::runtime_error if not enough space in buffer (current pos + required space >= buffer size)
         */
        void RequireSpace(size_t space) const;

        virtual void WriteUInt8(uint8_t value) = 0;
        virtual void WriteInt8(int8_t value) = 0;

        virtual void WriteUInt16(uint16_t value) = 0;
        virtual void WriteInt16(int16_t value) = 0;

        virtual void WriteUInt32(uint32_t value) = 0;
        virtual void WriteInt32(int32_t value) = 0;

        virtual uint8_t ReadUInt8() const = 0;
        virtual int8_t ReadInt8() const = 0;

        virtual uint16_t ReadUInt16() const = 0;
        virtual int16_t ReadInt16() const = 0;

        virtual uint32_t ReadUInt32() const = 0;
        virtual int32_t ReadInt32() const = 0;
    };

    /**
     * @class BinaryWalker
     * @brief Simple abstraction under "bytes stream"
     */
    class BinaryWalker final : public IBaseStreamWalker
    {
    private:
        uint8_t* m_buffer { nullptr };

    public:
        BinaryWalker() = default;
        BinaryWalker(uint8_t* buffer, size_t size);

        BinaryWalker(const BinaryWalker& copy) noexcept;
        BinaryWalker(BinaryWalker&& move) noexcept;
        BinaryWalker& operator=(const BinaryWalker& copy) noexcept;
        BinaryWalker& operator=(BinaryWalker&& move) noexcept;

        operator bool() const;

        /**
         * Read single value and move caret to sizeof(T)
         * @tparam T type of entity (arithmetic types only!)
         * @return value
         */
        template <typename T> T Read() const requires std::is_arithmetic_v<T>
        {
            if constexpr (std::is_same_v<T, uint8_t>) return ReadUInt8();
            else if constexpr (std::is_same_v<T, int8_t>) return ReadInt8();
            else if constexpr (std::is_same_v<T, uint16_t>) return ReadUInt16();
            else if constexpr (std::is_same_v<T, int16_t>) return ReadInt16();
            else if constexpr (std::is_same_v<T, uint32_t>) return ReadUInt32();
            else if constexpr (std::is_same_v<T, int32_t>) return ReadInt32();
            else
            {
                RequireSpace(sizeof(T));

                char* ptr = (&((char*)m_buffer)[m_offset]);

                m_offset += sizeof(T);
                return *((T*)ptr);
            }
        }

        template <typename T> void Write(const T& value) requires std::is_arithmetic_v<T>
        {
            if constexpr (std::is_same_v<T, uint8_t>) WriteUInt8(value);
            else if constexpr (std::is_same_v<T, int8_t>) WriteInt8(value);
            else if constexpr (std::is_same_v<T, uint16_t>) WriteUInt16(value);
            else if constexpr (std::is_same_v<T, int16_t>) WriteInt16(value);
            else if constexpr (std::is_same_v<T, uint32_t>) WriteUInt32(value);
            else if constexpr (std::is_same_v<T, int32_t>) WriteInt32(value);
            else {
                RequireSpace(sizeof(T));

                char* ptr = (&((char*)m_buffer)[m_offset]);
                *(T*)(ptr) = value;

                m_offset += sizeof(T);
            }
        }

        /**
         * Read multiple values of given type into STL container
         * @tparam T type of STL container
         * @param container reference to STL container with result
         */
        template <typename T>
        void ReadArray(T& container) const requires (STLContainer<T>)
        {
            using IT = typename T::value_type;

            if (!m_buffer || m_offset >= m_size)
                throw std::out_of_range { fmt::format("Trying to read {:X} byte from buffer with size is {:X} bytes", m_offset, m_size) };

            size_t total = container.size();
            for (size_t i = 0; i < total; i++)
            {
                container[i] = Read<IT>();
            }
        }

        template <typename T>
        void ReadArray(T* buffer, size_t size) const
        {
            if (!m_buffer || m_offset + sizeof(T) * size >= m_size)
                throw std::out_of_range { "Unable to read buffer. Not enough bytes" };

            const size_t offset = sizeof(T) * size;
            std::memcpy(buffer, m_buffer + m_offset, offset);

            m_offset += offset;
        }

        /**
         * Read multiple values of given type into non managed container (C array)
         * @tparam T entity type
         * @tparam S entities count
         * @param container pointer to the storage
         */
        template <typename T, size_t S>
        void ReadArray(T* container) const
        {
            std::array<T, S> buffer;
            ReadArray(buffer);
            std::memcpy(container, buffer.data(), buffer.size());
        }

        template <typename T>
        void WriteArray(const T& container) requires (STLContainer<T>)
        {
            using IT = typename T::value_type;

            for (const auto& entity : container)
            {
                Write<IT>(entity);
            }
        }

        template <typename T, size_t S>
        void WriteArray(const T* container)
        {
            for (size_t i = 0; i < S; i++)
            {
                Write<T>(container[i]);
            }
        }

        template <typename T>
        void WriteArray(const T* buffer, size_t size)
        {
            if (!m_buffer || m_offset + sizeof(T) * size >= m_size)
                throw std::out_of_range { "Unable to read buffer. Not enough bytes" };

            std::memcpy(m_buffer, buffer + (sizeof(T) * size), size);

            m_offset += (sizeof(T) * size);
        }

        // Interface
        void WriteUInt8(uint8_t value) override;
        void WriteInt8(int8_t value) override;

        void WriteUInt16(uint16_t value) override;
        void WriteInt16(int16_t value) override;

        void WriteUInt32(uint32_t value) override;
        void WriteInt32(int32_t value) override;

        uint8_t ReadUInt8() const override;
        int8_t ReadInt8() const override;

        uint16_t ReadUInt16() const override;
        int16_t ReadInt16() const override;

        uint32_t ReadUInt32() const override;
        int32_t ReadInt32() const override;

        // Custom
        std::string ReadZString(int limit = 1024) const;
    };
}