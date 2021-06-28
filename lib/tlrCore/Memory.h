// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021 Darby Johnston
// All rights reserved.

#pragma once

#include <tlrCore/Util.h>

#include <iostream>
#include <string>
#include <vector>

namespace tlr
{
    //! Memory.
    namespace memory
    {
        //! Endian type.
        enum class Endian
        {
            MSB, //!< Most siginificant byte first
            LSB, //!< Least significant byte first

            Count,
            First = MSB
        };
        TLR_ENUM(Endian);

        //! Get the current machine's endian.
        constexpr Endian getEndian() noexcept;

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

    TLR_ENUM_SERIALIZE(memory::Endian);
}

#include <tlrCore/MemoryInline.h>
