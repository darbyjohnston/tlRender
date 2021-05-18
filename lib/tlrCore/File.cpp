// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021 Darby Johnston
// All rights reserved.

#include <tlrCore/File.h>

#include <tlrCore/String.h>

extern "C"
{
#include <fseq.h>
}

namespace tlr
{
    namespace file
    {
        bool isAbsolute(const std::string& path)
        {
            const std::size_t size = path.size();
            if (size > 0 && '/' == path[0])
            {
                return true;
            }
            else if (size > 0 && '\\' == path[0])
            {
                return true;
            }
            if (size > 1 && path[0] >= 'A' && path[0] <= 'Z' && ':' == path[1])
            {
                return true;
            }
            return false;
        }

        std::string normalize(const std::string& path)
        {
            std::string out;
            for (auto i : path)
            {
                out.push_back('\\' == i ? '/' : i);
            }
            return out;
        }

        void split(
            const std::string& fileName,
            std::string* path,
            std::string* baseName,
            std::string* number,
            std::string* extension)
        {
            struct FSeqFileName f;
            fseqFileNameInit(&f);
            fseqFileNameSplit(fileName.c_str(), &f, string::cBufferSize);
            *path = f.path;
            if (baseName)
            {
                *baseName = f.base;
            }
            if (number)
            {
                *number = f.number;
            }
            if (extension)
            {
                *extension = f.extension;
            }
            fseqFileNameDel(&f);
        }
    }
}