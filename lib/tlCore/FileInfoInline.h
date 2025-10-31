// SPDX-License-Identifier: BSD-3-Clause
// Copyright Contributors to the tlRender project.

namespace tl
{
    namespace file
    {
        inline const Path& FileInfo::getPath() const
        {
            return _path;
        }

        inline Type FileInfo::getType() const
        {
            return _type;
        }

        inline uint64_t FileInfo::getSize() const
        {
            return _size;
        }

        inline int FileInfo::getPermissions() const
        {
            return _permissions;
        }

        inline time_t FileInfo::getTime() const
        {
            return _time;
        }
    }
}
