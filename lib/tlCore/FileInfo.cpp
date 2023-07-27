// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2023 Darby Johnston
// All rights reserved.

#include <tlCore/FileInfo.h>

#include <tlCore/Error.h>
#include <tlCore/String.h>

#include <fseq.h>

#include <algorithm>
#include <array>
#include <functional>

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

        TLRENDER_ENUM_IMPL(
            ListSort,
            "Name",
            "Size",
            "Time");
        TLRENDER_ENUM_SERIALIZE_IMPL(ListSort);

        bool ListOptions::operator == (const ListOptions& other) const
        {
            return
                sort == other.sort &&
                reverseSort == other.reverseSort &&
                sortDirectoriesFirst == other.sortDirectoriesFirst &&
                dotAndDotDotDirs == other.dotAndDotDotDirs &&
                dotFiles == other.dotFiles &&
                sequence == other.sequence &&
                negativeNumbers == other.negativeNumbers &&
                maxNumberDigits == other.maxNumberDigits;
        }

        bool ListOptions::operator != (const ListOptions& other) const
        {
            return !(*this == other);
        }

        std::vector<FileInfo> list(const std::string& path, const ListOptions& options)
        {
            std::vector<FileInfo> out;

            FSeqDirOptions dirOptions;
            fseqDirOptionsInit(&dirOptions);
            dirOptions.dotAndDotDotDirs = options.dotAndDotDotDirs;
            dirOptions.dotFiles = options.dotFiles;
            dirOptions.sequence = options.sequence;
            dirOptions.fileNameOptions.negativeNumbers = options.negativeNumbers;
            dirOptions.fileNameOptions.maxNumberDigits = options.maxNumberDigits;

            FSeqBool error = FSEQ_FALSE;
            auto dirList = fseqDirList(path.c_str(), &dirOptions, &error);
            if (FSEQ_FALSE == error)
            {
                const std::string directory = appendSeparator(path);
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

            std::function<int(const FileInfo& a, const FileInfo& b)> sort;
            switch (options.sort)
            {
            case ListSort::Name:
                sort = [](const FileInfo& a, const FileInfo& b)
                {
                    return a.getPath().get() < b.getPath().get();
                };
                break;
            case ListSort::Size:
                sort = [](const FileInfo& a, const FileInfo& b)
                {
                    return a.getSize() < b.getSize();
                };
                break;
            case ListSort::Time:
                sort = [](const FileInfo& a, const FileInfo& b)
                {
                    return a.getTime() < b.getTime();
                };
                break;
            }
            if (sort)
            {
                if (options.reverseSort)
                {
                    std::sort(out.rbegin(), out.rend(), sort);
                }
                else
                {
                    std::sort(out.begin(), out.end(), sort);
                }
            }
            if (options.sortDirectoriesFirst)
            {
                std::stable_sort(
                    out.begin(),
                    out.end(),
                    [](const FileInfo& a, const FileInfo& b)
                    {
                        return
                            static_cast<size_t>(a.getType()) >
                            static_cast<size_t>(b.getType());
                    });
            }

            return out;
        }
    }
}
