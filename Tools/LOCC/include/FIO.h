#pragma once

#include <LOCC.h>

namespace LOCC
{
    struct FIO
    {
        static bool HasFile(std::string_view path);

        static std::unique_ptr<char[]> ReadFile(std::string_view path, size_t& bufferSize);
        static nlohmann::json ReadJson(std::string_view path);

        static bool WriteFile(std::string_view path, const char* buffer, size_t bufferSize);
        static bool WriteFile(std::string_view path, const nlohmann::json& json, int indent = -1);
    };
}