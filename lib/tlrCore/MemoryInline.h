// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2022 Darby Johnston
// All rights reserved.

#include <cstdint>

namespace tlr
{
    namespace memory
    {
        inline Endian getEndian() noexcept
        {
            const uint32_t tmp = 1;
            return *(reinterpret_cast<const uint8_t*>(&tmp)) ? Endian::LSB : Endian::MSB;
        }

        constexpr Endian opposite(Endian in) noexcept
        {
            return Endian::MSB == in ? Endian::LSB : Endian::MSB;
        }
    }
}
