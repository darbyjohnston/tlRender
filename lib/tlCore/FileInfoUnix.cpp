// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2022 Darby Johnston
// All rights reserved.

#include <tlCore/FileInfo.h>

#include <sys/stat.h>
#include <sys/types.h>

#include <cstring>

#if defined(__APPLE__)
//! \bug OS X doesn't have stat64?
#define _STAT struct ::stat
#define _STAT_FNC    ::stat
#else
#define _STAT struct ::stat64
#define _STAT_FNC    ::stat64
#endif

namespace tl
{
    namespace file
    {
        bool FileInfo::_stat(std::string* error)
        {
            _STAT info;
            memset(&info, 0, sizeof(_STAT));
            if (_STAT_FNC(_path.get().c_str(), &info) != 0)
            {
            	return false;
            }
            
            _exists = true;
            if (S_ISDIR(info.st_mode))
            {
            	_type = Type::Directory;
            }
            _size = info.st_size;
            _permissions |= (info.st_mode & S_IRUSR) ? static_cast<int>(Permissions::Read)  : 0;
            _permissions |= (info.st_mode & S_IWUSR) ? static_cast<int>(Permissions::Write) : 0;
            _permissions |= (info.st_mode & S_IXUSR) ? static_cast<int>(Permissions::Exec)  : 0;
            _time = info.st_mtime;
            
            return true;
        }
    }
}
