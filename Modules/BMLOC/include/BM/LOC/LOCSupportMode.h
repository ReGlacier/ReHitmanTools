#pragma once

namespace BM::LOC
{
    enum class LOCSupportMode
    {
        Hitman_BloodMoney = 0x0004,
        Hitman_Contracts  = 0x0003,
        Hitman_2SA        = 0x0002,
        Hitman_A47        = 0x0001,
        // Generic mode
        Generic = Hitman_BloodMoney
    };
}