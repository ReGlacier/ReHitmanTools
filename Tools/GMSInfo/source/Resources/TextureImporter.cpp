#include <Resources/TextureImporter.h>

#include <stdexcept>

namespace ReGlacier
{
    std::unique_ptr<char []> && TextureImporter::RecognizeTextureOptions(
            std::unique_ptr<char[]>&& buffer,
            size_t bufferSize,
            int& width,
            int& height,
            bool& recognized)
    {
        throw std::runtime_error { "NOT IMPLEMENTED" };
    }
}