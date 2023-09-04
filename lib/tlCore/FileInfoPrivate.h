// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2023 Darby Johnston
// All rights reserved.

#pragma once

#include <tlCore/FileInfo.h>

namespace tl
{
    namespace file
    {
        void _list(
            const std::string&,
            std::vector<FileInfo>&,
            const ListOptions & = ListOptions());
    }
}
