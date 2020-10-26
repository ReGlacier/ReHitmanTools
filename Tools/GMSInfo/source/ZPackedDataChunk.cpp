#include <ZPackedDataChunk.h>

#include <cstdlib>
#include <cstring>

extern "C" {
#include <zlib.h>
}

namespace ReGlacier
{
    static constexpr int kDataOffset     = 0x9;
    static constexpr int kZlibWindowBits = -15;
    static constexpr int kZlibStreamSize = 0x38;
    static constexpr int kZlibFlushValue = 4;

    ZPackedDataChunk::ZPackedDataChunk()
        : m_bufferWasFreed(false)
        , field_1(false)
        , field_2(false)
        , field_3(false)
        , m_buffer(nullptr)
        , m_bufferSize(0)
        , m_uncompressedSize(0)
        , field_10(0)
        , m_isUncompressedAlready(0)
    {}

    ZPackedDataChunk::~ZPackedDataChunk()
    {
        if (m_buffer)
        {
            if (!m_bufferWasFreed)
            {
                std::free(m_buffer);
            }

            m_buffer = nullptr;
            m_bufferSize = 0;
            m_uncompressedSize = 0;
            m_isUncompressedAlready = 0;
        }
    }

    bool ZPackedDataChunk::unzip(void* outputBuffer, size_t outputBufferSize)
    {
        if (m_isUncompressedAlready == 1)
        {
            auto uncompressedSize = m_uncompressedSize;
            if (outputBufferSize <= uncompressedSize)
            {
                uncompressedSize = outputBufferSize;
            }

            std::memcpy(outputBuffer, m_buffer + kDataOffset, uncompressedSize);
            return false;
        }

        char* buffer = reinterpret_cast<char*>(m_buffer);

        z_stream zStream;
        zStream.avail_in  = m_bufferSize;                                    // how much bytes ready to decompress
        zStream.next_in   = reinterpret_cast<Bytef*>(buffer + 1);            // entry buffer (compressed)
        zStream.next_out  = reinterpret_cast<Bytef*>(outputBuffer);          // out stream
        zStream.avail_out = outputBufferSize;                                // chunk size (for this - full stream size)
        zStream.zalloc    = Z_NULL;                                          // custom allocator [nope]
        zStream.zfree     = Z_NULL;                                          // custom allocator [nope]

        if ( !inflateInit2_(&zStream, kZlibWindowBits, "1.1.3", kZlibStreamSize) )// inflateInit2_ (stream, windowBits, version, stream size)
        {
            auto streamDecompressionResult = inflate(&zStream, kZlibFlushValue);
            inflateEnd(&zStream);
            if (streamDecompressionResult == Z_STREAM_END || streamDecompressionResult == Z_BUF_ERROR && !zStream.avail_out)
            {
                return false;
            }
        }

        return true;
    }
}