// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2022 Darby Johnston
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

        std::vector<FileInfo> dirList(const std::string& path, const file::PathOptions& pathOptions)
        {
            std::vector<FileInfo> out;
            FSeqDirOptions dirOptions;
            fseqDirOptionsInit(&dirOptions);
            dirOptions.fileNameOptions.maxNumberDigits = pathOptions.maxNumberDigits;
            FSeqBool error = FSEQ_FALSE;
            auto dirList = fseqDirList(path.c_str(), &dirOptions, &error);
            if (FSEQ_FALSE == error)
            {
                std::string directory = path;
                const size_t size = directory.size();
                if (size > 0 && !('/' == directory[size - 1] || '\\' == directory[size - 1]))
                {
                    directory += '/';
                }
                for (auto entry = dirList; entry; entry = entry->next)
                {
                    out.push_back(FileInfo(Path(
                        directory,
                        entry->fileName.base,
                        entry->fileName.number,
                        entry->framePadding,
                        entry->fileName.extension)));
                }
            }
            fseqDirListDel(dirList);
            return out;
        }
    }
}
