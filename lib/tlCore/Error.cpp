// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2022 Darby Johnston
// All rights reserved.

#include <tlCore/Error.h>

namespace tl
{
    namespace core
    {
        ParseError::ParseError() :
            invalid_argument("Cannot parse value")
        {}
    }
}
