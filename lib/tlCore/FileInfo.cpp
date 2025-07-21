// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2025 Darby Johnston
// All rights reserved.

#include <tlCore/FileInfoPrivate.h>

#include <feather-tk/core/Error.h>
#include <feather-tk/core/String.h>

#include <algorithm>
#include <array>
#include <functional>
#include <sstream>

namespace tl
{
    namespace file
    {
        FEATHER_TK_ENUM_IMPL(
            Type,
            "File",
            "Directory");

        FileInfo::FileInfo()
        {}

        FileInfo::FileInfo(const Path& path) :
            _path(path)
        {
            std::string error;
            _stat(&error);
        }

        void FileInfo::sequence(const FileInfo& value)
        {
            if (!_path.getNumber().empty() &&
                !value._path.getNumber().empty() &&
                (_path.getPadding() == value._path.getPadding() ||
                 _path.getPadding() == value._path.getNumber().size() ||
                 _path.getNumber().size() == value._path.getPadding()))
            {
                _path.setPadding(std::max(_path.getPadding(), value._path.getPadding()));
                feather_tk::RangeI sequence = _path.getSequence();
                const feather_tk::RangeI& otherSequence = value._path.getSequence();
                sequence = feather_tk::expand(sequence, otherSequence.min());
                sequence = feather_tk::expand(sequence, otherSequence.max());
                _path.setSequence(sequence);
                _size += value._size;
                _permissions = std::min(_permissions, value._permissions);
                _time = std::max(_time, value._time);
            }
        }

        FEATHER_TK_ENUM_IMPL(
            ListSort,
            "Name",
            "Extension",
            "Size",
            "Time");

        bool ListOptions::operator == (const ListOptions& other) const
        {
            return
                sort == other.sort &&
                reverseSort == other.reverseSort &&
                sortDirectoriesFirst == other.sortDirectoriesFirst &&
                dotAndDotDotDirs == other.dotAndDotDotDirs &&
                dotFiles == other.dotFiles &&
                extensions == other.extensions &&
                sequence == other.sequence &&
                sequenceExtensions == other.sequenceExtensions &&
                negativeNumbers == other.negativeNumbers &&
                maxNumberDigits == other.maxNumberDigits;
        }

        bool ListOptions::operator != (const ListOptions& other) const
        {
            return !(*this == other);
        }

        bool listFilter(const std::string& fileName, const ListOptions& options)
        {
            bool out = false;

            if (!options.dotAndDotDotDirs &&
                1 == fileName.size() &&
                '.' == fileName[0])
            {
                out = true;
            }
            else if (!options.dotAndDotDotDirs &&
                2 == fileName.size() &&
                '.' == fileName[0] &&
                '.' == fileName[1])
            {
                out = true;
            }
            else if (!options.dotFiles &&
                fileName.size() > 0 &&
                '.' == fileName[0])
            {
                out = true;
            }

            if (!out && !options.extensions.empty())
            {
                const size_t fileNameSize = fileName.size();
                bool match = false;
                for (const auto& extension : options.extensions)
                {
                    const size_t size = extension.size();
                    if (size < fileNameSize &&
                        feather_tk::compare(
                            fileName.substr(fileNameSize - size, size),
                            extension,
                            feather_tk::CaseCompare::Insensitive))
                    {
                        match = true;
                    }
                }
                out = !match;
            }

            return out;
        }
        
        void listSequence(
            const std::string& path,
            const std::string& fileName,
            std::vector<FileInfo>& out,
            const ListOptions& options)
        {
            PathOptions pathOptions;
            pathOptions.maxNumberDigits =
                options.sequence ?
                options.maxNumberDigits :
                0;
            const Path p(path, fileName, pathOptions);
            const FileInfo f(p);
            bool sequence = false;
            if (options.sequence &&
                !p.getNumber().empty() &&
                f.getType() != Type::Directory)
            {
                bool sequenceExtension = true;
                if (!options.sequenceExtensions.empty())
                {
                    const std::string extension = p.getExtension();
                    sequenceExtension = std::find_if(
                        options.sequenceExtensions.begin(),
                        options.sequenceExtensions.end(),
                        [extension](const std::string& value)
                        {
                            return feather_tk::compare(
                                value,
                                extension,
                                feather_tk::CaseCompare::Insensitive);
                        }) != options.sequenceExtensions.end();
                }
                if (sequenceExtension)
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
            }
            if (!sequence)
            {
                out.push_back(f);
            }
        }
        
        void list(
            const std::string& path,
            std::vector<FileInfo>& out,
            const ListOptions& options)
        {
            out.clear();

            _list(path, out, options);
            
            std::function<int(const FileInfo& a, const FileInfo& b)> sort;
            switch (options.sort)
            {
            case ListSort::Name:
                sort = [](const FileInfo& a, const FileInfo& b)
                {
                    return a.getPath().get() < b.getPath().get();
                };
                break;
            case ListSort::Extension:
                sort = [](const FileInfo& a, const FileInfo& b)
                {
                    return a.getPath().getExtension() < b.getPath().getExtension();
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
            default: break;
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
        }
    }
}
