#pragma once

#include <cstdint>

namespace BM::LOC
{
    enum TreeNodeType : int8_t {
        VALUE_OR_DATA      = 0x0,
        NODE_WITH_CHILDREN = 0x10
    };

    enum HBM_MissionObjectiveType : char
    {
        HBM_Target = 'T',
        HBM_Retrieve = 'R',
        HBM_Escape = 'E',
        HBM_Dispose = 'D',
        HBM_Protect = 'P',
        HBM_Optional = 'O'
    };
}