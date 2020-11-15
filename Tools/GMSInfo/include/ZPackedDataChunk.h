#pragma once

namespace ReGlacier
{
    struct ZPackedDataChunk
    {
        bool m_bufferWasFreed; ///+0
        bool field_1; ///+1
        bool field_2; //+2
        bool field_3; //+3
        int *m_buffer;  ///+4
        int m_bufferSize; ///+8
        int m_uncompressedSize; ///+C
        int field_10;///+10
        int m_isUncompressedAlready;///+14

        // API
        ZPackedDataChunk();
        ~ZPackedDataChunk();

        bool unzip(void* outputBuffer, size_t outputBufferSize);
    };
}