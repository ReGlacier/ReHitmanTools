#include <FIO.h>

namespace LOCC
{
    bool FIO::HasFile(std::string_view path)
    {
        FILE* fp = fopen(path.data(), "rb");
        if (!fp) return false;
        fclose(fp);
        return true;
    }

    std::unique_ptr<char[]> FIO::ReadFile(std::string_view path, size_t& bufferSize)
    {
        std::unique_ptr<char[]> result = nullptr;

        FILE* fp = fopen(path.data(), "rb");
        if (!fp)
        {
            spdlog::error("FIO::ReadFile| Failed to open file {} to read binary contents!", path);
            return nullptr;
        }

        // Recognize the size
        fseek(fp, 0L, SEEK_END);
        bufferSize = ftell(fp);
        rewind(fp);

        if (bufferSize == 0)
        {
            spdlog::warn("FIO::ReadFile| File {} is empty. Nothing to return/allocate", path);
            return nullptr; // Empty file - no buffer
        }

        result = std::make_unique<char[]>(bufferSize);
        auto readBytes = fread(result.get(), sizeof(char), bufferSize, fp);
        fclose(fp);

        if (readBytes != bufferSize)
        {
            spdlog::error("FIO::ReadFile| Failed to read file {}. Requested {} bytes, got {} bytes {}", path, bufferSize, readBytes);
            bufferSize = 0;
            return nullptr;
        }

        return result;
    }

    nlohmann::json FIO::ReadJson(std::string_view path)
    {
        std::ifstream file { path.data(), std::ifstream::in };
        if (!file.good())
        {
            spdlog::error("FIO::ReadJson| Failed to read file {}", path);
            return nlohmann::json {};
        }

        try
        {
            nlohmann::json j;
            file >> j;
            file.close();
            return j;
        }catch (const nlohmann::json::exception& ex)
        {
            spdlog::error("FIO::ReadJson| Failed to parse json from file {}. Reason: {}", path, ex.what());
        }
        catch (const std::exception& STDERR)
        {
            spdlog::error("FIO::ReadJson| Failed to parse json from file {}. STD Reason: {}", path, STDERR.what());
        }

        return nlohmann::json {};
    }

    bool FIO::WriteFile(std::string_view path, const char* buffer, size_t bufferSize)
    {
        FILE* fp = fopen(path.data(), "wb");
        if (!fp) {
            spdlog::error("FIO::WriteFile| Failed to open file {} to write binary contents", path);
            return false;
        }

        auto writtenBytes = fwrite(buffer, sizeof(char), bufferSize, fp);
        fclose(fp);

        if (writtenBytes != bufferSize)
        {
            spdlog::error("FIO::WriteFile| Failed to write buffer of size {} into file {}. Actually written {} bytes", path, bufferSize, writtenBytes);
            return false;
        }

        return true;
    }

    bool FIO::WriteFile(std::string_view path, const nlohmann::json& json, int indent)
    {
        std::string buffer = json.dump(indent);
        return FIO::WriteFile(path, buffer.data(), buffer.size());
    }
}