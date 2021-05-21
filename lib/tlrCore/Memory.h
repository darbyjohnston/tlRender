// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021 Darby Johnston
// All rights reserved.

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

        //! Get the current machine's endian.
        Endian getEndian() noexcept;

        //! Get the opposite of the given endian.
        Endian opposite(Endian) noexcept;
    }
}

#include <tlrCore/MemoryInline.h>
