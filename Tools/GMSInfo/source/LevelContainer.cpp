#include <LevelContainer.h>
#include <spdlog/spdlog.h>

extern "C" {
#include <unzip.h>
}

namespace ReGlacier
{
    LevelContainer::LevelContainer(void* zipPtr) : m_zip(zipPtr)
    {}

    std::unique_ptr<uint8_t[]> LevelContainer::Read(std::string_view path, size_t& bufferSize)
    {
        bufferSize = 0;
        unzFile zip = m_zip;

        if (unzGoToFirstFile(zip) != UNZ_OK)
        {
            spdlog::error("Failed to open {} file in level archive", path);
            return nullptr;
        }

        do {
            static constexpr int kMaxFileNameLength = 256;
            char fileName[kMaxFileNameLength] = { 0 };
            unz_file_info fileInfo;

            unsigned int ret = unzGetCurrentFileInfo(
                    zip,
                    &fileInfo,
                    fileName,
                    kMaxFileNameLength,
                    nullptr,0, nullptr, 0);
            if (ret != UNZ_OK)
            {
                spdlog::error("LevelContainer::Read| unzGetCurrentFileInfo() failed with error code {}", ret);
                return nullptr;
            }

            if (std::string_view(fileName) == path)
            {
                ret = unzOpenCurrentFile(zip);
                if (ret != UNZ_OK)
                {
                    spdlog::error("LevelContainer::Read| Unable to open file {}. unzOpenCurrentFile() failed with error code {}", path, ret);
                    return nullptr;
                }

                auto buffer = std::make_unique<uint8_t[]>(fileInfo.uncompressed_size);
                int readBytes = unzReadCurrentFile(zip, buffer.get(), fileInfo.uncompressed_size);

                if (readBytes < 0)
                {
                    spdlog::error("LevelContainer::Read| unzReadCurrentFile() failed for file {} with error code {}", path, readBytes);
                    return nullptr;
                }

                bufferSize = readBytes;

                if (fileInfo.uncompressed_size != bufferSize)
                {
                    spdlog::warn("LevelContainer::Read| File read operation got wrong buffer size. Await {} got {}", fileInfo.uncompressed_size, bufferSize);
                }

                return std::move(buffer);
            }

            ret = unzGoToNextFile(zip);
            if (ret != UNZ_OK)
            {
                spdlog::warn("LevelContainer::Read| File {} not found in level archive. unzGoToNextFile() = {}", path, ret);
                return nullptr;
            }
        }
        while (true);

        return nullptr;
    }
}