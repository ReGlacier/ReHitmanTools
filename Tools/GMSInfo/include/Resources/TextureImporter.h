#pragma once

#include <memory>

namespace ReGlacier
{
    class TextureImporter
    {
    public:
        static std::unique_ptr<char[]>&& RecognizeTextureOptions(std::unique_ptr<char[]>&& buffer, size_t bufferSize, int& width, int& height, bool& recognized);
    };
}