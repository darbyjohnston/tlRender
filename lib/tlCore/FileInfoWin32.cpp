// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2023 Darby Johnston
// All rights reserved.

#include <tlCore/FileInfoPrivate.h>

#include <tlCore/String.h>

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif // WIN32_LEAN_AND_MEAN
#ifndef NOMINMAX
#define NOMINMAX
#endif // NOMINMAX
#include <sys/stat.h>
#include <windows.h>

namespace tl
{
    namespace file
    {
        bool FileInfo::_stat(std::string* error)
        {
            struct _stati64 info;
            memset(&info, 0, sizeof(struct _stati64));
            if (_wstati64(string::toWide(_path.get()).c_str(), &info) != 0)
            {
                if (error)
                {
                    char tmp[string::cBufferSize] = "";
                    strerror_s(tmp, string::cBufferSize, errno);
                    *error = tmp;
                }
                return false;
            }

            _exists = true;
            if (info.st_mode & _S_IFDIR)
            {
                _type = Type::Directory;
            }
            _size = info.st_size;
            _permissions |= (info.st_mode & _S_IREAD)  ? static_cast<int>(Permissions::Read)  : 0;
            _permissions |= (info.st_mode & _S_IWRITE) ? static_cast<int>(Permissions::Write) : 0;
            _permissions |= (info.st_mode & _S_IEXEC)  ? static_cast<int>(Permissions::Exec)  : 0;
            _time = info.st_mtime;

            return true;
        }

        void _list(
            const std::string& path,
            std::vector<FileInfo>& out,
            const ListOptions& options)
        {
            std::string glob = appendSeparator(path);
            glob.push_back('*');

            WIN32_FIND_DATAW ffd;
            HANDLE hFind = FindFirstFileW(string::toWide(glob).c_str(), &ffd);
            if (INVALID_HANDLE_VALUE == hFind)
            {
                return;
            }

            do
            {
                const std::string fileName = string::fromWide(ffd.cFileName);
                
                bool filter = false;
                if (!options.dotAndDotDotDirs &&
                    1 == fileName.size() &&
                    '.' == fileName[0])
                {
                    filter = true;
                }
                else if (!options.dotAndDotDotDirs &&
                    2 == fileName.size() &&
                    '.' == fileName[0] &&
                    '.' == fileName[1])
                {
                    filter = true;
                }
                else if (!options.dotFiles &&
                    fileName.size() > 0 &&
                    '.' == fileName[0])
                {
                    filter = true;
                }

                if (!filter)
                {
                    const Path p(path, fileName);
                    const FileInfo f(p);

                    bool sequence = false;
                    if (options.sequence && !p.getNumber().empty())
                    {
                        for (auto& i : out)
                        {
                            if (i.getPath().sequence(p))
                            {
                                sequence = true;
                                i.sequence(f);
                                break;
                            }
                        }
                    }

                    if (!sequence)
                    {
                        out.push_back(f);
                    }
                }
            }
            while (FindNextFileW(hFind, &ffd) != 0);

            FindClose(hFind);
        }
    }
}
