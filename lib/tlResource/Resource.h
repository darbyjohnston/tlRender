// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2025 Darby Johnston
// All rights reserved.

#pragma once

#include <cstdint>
#include <string>
#include <vector>

namespace tl
{
    //! Get the list of icon resources.
    std::vector<std::string> getIconResources();

    //! Get an icon resource.
    std::vector<uint8_t> getIconResource(const std::string&);
}
