// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2025 Darby Johnston
// All rights reserved.

#pragma once

#include <ftk/Core/Util.h>

#include <string>

namespace tl
{
    //! URLs
    namespace url
    {
        //! Get the URL scheme.
        std::string scheme(const std::string&);

        //! Encode a URL.
        std::string encode(const std::string&);

        //! Decode a URL.
        std::string decode(const std::string&);
    }
}
