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

    /**
     * @class BinaryWalker
     * @brief Simple abstraction under "bytes stream"
     */
    class BinaryWalker final
    {
    private:
        std::unique_ptr<uint8_t[]> m_buffer;
        size_t m_size;
        mutable size_t m_offset { 0 };

    public:
        enum class SeekType : uint8_t
        {
            FROM_CURRENT_POSITION, ///< Append offset to current offset
            FROM_BEGIN, ///< Override current offset to new value
            FROM_END ///< Override current offset to m_size - offset
        };

        BinaryWalker(std::unique_ptr<uint8_t[]>&& buffer, size_t size);

        /**
         * Move internal offset to zero, equal to Seek(0, SeekType::FROM_BEGIN)
         */
        void Reset();

        /**
         * Read single value and move caret to sizeof(T)
         * @tparam T type of entity (arithmetic types only!)
         * @return value
         */
        template <typename T> T Read() const requires std::is_arithmetic_v<T>
        {
            if (!m_buffer || m_offset >= m_size)
                throw std::out_of_range { fmt::format("Trying to read {:X} byte from buffer with size is {:X} bytes", m_offset, m_size) };

            char* ptr = (&((char*)m_buffer.get())[m_offset]);

            m_offset += sizeof(T);
            return *((T*)ptr);
        }

        template <typename T> void Write(const T& value) requires std::is_arithmetic_v<T>
        {
            if (!m_buffer || m_offset >= m_size)
                throw std::out_of_range { fmt::format("Trying to read {:X} byte from buffer with size is {:X} bytes", m_offset, m_size) };

            char* ptr = (&((char*)m_buffer.get())[m_offset]);
            *(T*)(ptr) = value;

            m_offset += sizeof(T);
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

        /**
         * Move internal offset to passed value with passed rule
         * @param offset - new offset
         * @param seekType - new offset type
         */
        void Seek(size_t offset, SeekType seekType);

        /**
         * Move out holded pointer and reset internal state
         */
        std::unique_ptr<uint8_t[]>&& Take();
    };
}