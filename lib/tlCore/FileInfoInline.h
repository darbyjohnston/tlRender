// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2022 Darby Johnston
// All rights reserved.

namespace tl
{
    namespace file
    {
        inline const Path& FileInfo::getPath() const noexcept
        {
            return _path;
        }

        inline Type FileInfo::getType() const noexcept
        {
            return _type;
        }

        inline uint64_t FileInfo::getSize() const noexcept
        {
            return _size;
        }

        inline int FileInfo::getPermissions() const noexcept
        {
            return _permissions;
        }

        inline time_t FileInfo::getTime() const noexcept
        {
            return _time;
        }
    }
}
