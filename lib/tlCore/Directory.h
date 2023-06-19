// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2023 Darby Johnston
// All rights reserved.

#pragma once

#include <tlCore/FileInfo.h>

#include <iostream>

namespace tl
{
    namespace file
    {
        //! Directory list options.
        struct ListOptions
        {
            bool   dotAndDotDotDirs = false;
            bool   dotFiles         = false;
            bool   sequence         = true;
            bool   negativeNumbers  = false;
            size_t maxNumberDigits  = 9;

            bool operator == (const ListOptions&) const;
            bool operator != (const ListOptions&) const;
        };

        //! Get the contents of the given directory.
        std::vector<FileInfo> list(
            const std::string&,
            const ListOptions& = ListOptions());
    }
}
