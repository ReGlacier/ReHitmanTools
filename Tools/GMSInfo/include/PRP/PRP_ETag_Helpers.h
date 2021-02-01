#pragma once

#include <string_view>
#include <cstdint>

#include <PRP/PRPTypes.h>

namespace ReGlacier
{
    struct PRP_ETag_Helpers
    {
        static PRP_ETag FromByte(uint8_t byte);
        static std::string_view ToString(PRP_ETag tag);
    };
}