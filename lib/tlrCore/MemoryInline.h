// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021 Darby Johnston
// All rights reserved.

namespace tlr
{
    namespace memory
    {
        inline Endian getEndian() noexcept
        {
            const int tmp = 1;
            const uint8_t* const p = reinterpret_cast<const uint8_t*>(&tmp);
            const Endian endian = *p ? Endian::LSB : Endian::MSB;
            return endian;
        }

        inline Endian opposite(Endian in) noexcept
        {
            return Endian::MSB == in ? Endian::LSB : Endian::MSB;
        }
    }
}