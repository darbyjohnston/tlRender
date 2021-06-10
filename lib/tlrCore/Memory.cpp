// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021 Darby Johnston
// All rights reserved.

#include <tlrCore/Memory.h>

#include <tlrCore/Error.h>
#include <tlrCore/String.h>

#include <algorithm>
#include <array>
#include <string>

namespace tlr
{
    namespace memory
    {
        TLR_ENUM_LABEL_IMPL(
            Endian,
            "MSB",
            "LSB");
    }

    TLR_ENUM_SERIALIZE_IMPL(memory, Endian);
}
