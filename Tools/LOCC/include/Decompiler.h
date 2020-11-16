#pragma once

#include <string_view>
#include <ToolExitCodes.h>

namespace LOCC
{
    ToolExitCodes Decompile(std::string_view from, std::string_view to);
}