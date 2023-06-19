// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2023 Darby Johnston
// All rights reserved.

#include <tlCore/FileInfo.h>

#include <tlCore/Error.h>
#include <tlCore/String.h>

#include <fseq.h>

#include <algorithm>
#include <array>

namespace tl
{
    namespace file
    {
        TLRENDER_ENUM_IMPL(
            Type,
            "File",
            "Directory");
        TLRENDER_ENUM_SERIALIZE_IMPL(Type);

        FileInfo::FileInfo()
        {}

        FileInfo::FileInfo(const Path& path) :
            _path(path)
        {
            std::string error;
            _stat(&error);
        }
    }
}
