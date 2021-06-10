// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021 Darby Johnston
// All rights reserved.

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
        TLR_ENUM_LABEL(Endian);

        //! Get the current machine's endian.
        Endian getEndian() noexcept;

        //! Get the opposite of the given endian.
        Endian opposite(Endian) noexcept;
    }

    TLR_ENUM_SERIALIZE(memory::Endian);
}

#include <tlrCore/MemoryInline.h>
