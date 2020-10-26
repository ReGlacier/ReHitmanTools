#pragma once

#include <memory>
#include <string>

namespace ReGlacier
{
    class LevelContainer
    {
        void* m_zip;
    public:
        explicit LevelContainer(void* zipPtr);

        std::unique_ptr<uint8_t[]> Read(std::string_view path, size_t& bufferSize);
    };
}