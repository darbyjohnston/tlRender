// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021 Darby Johnston
// All rights reserved.

#pragma once

#include <stdexcept>

namespace tlr
{
    class ParseError : public std::invalid_argument
    {
    public:
        ParseError() :
            invalid_argument("Cannot parse value")
        {}
    };
}
