// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021 Darby Johnston
// All rights reserved.

#include <cstdint>

namespace tlr
{
    namespace memory
    {
        constexpr Endian getEndian() noexcept
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