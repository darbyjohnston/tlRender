// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2022 Darby Johnston
// All rights reserved.

#pragma once

#include <tlCore/Util.h>

#include <iostream>
#include <string>
#include <vector>

namespace tl
{
    //! Memory.
    namespace memory
    {
        constexpr size_t kilobyte = 1024; //!< The number of bytes in a kilobyte
        constexpr size_t megabyte = kilobyte * 1024; //!< The number of bytes in a megabyte
        constexpr size_t gigabyte = megabyte * 1024; //!< The number of bytes in a gigabyte
        constexpr size_t terabyte = gigabyte * 1024; //!< The number of bytes in a terabyte

        //! Endian type.
        enum class Endian
        {
            MSB, //!< Most siginificant byte first
            LSB, //!< Least significant byte first

            Count,
            First = MSB
        };
        TLRENDER_ENUM(Endian);
        TLRENDER_ENUM_SERIALIZE(Endian);

        //! Get the current machine's endian.
        Endian getEndian() noexcept;

        //! Get the opposite of the given endian.
        constexpr Endian opposite(Endian) noexcept;

        //! Convert the endianness of a block of memory in place.
        void endian(
            void*  in,
            size_t size,
            size_t wordSize) noexcept;

        //! Convert the endianness of a block of memory.
        void endian(
            const void* in,
            void*       out,
            size_t      size,
            size_t      wordSize) noexcept;
    }
}

#include <tlCore/MemoryInline.h>
