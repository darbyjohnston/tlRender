// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021 Darby Johnston
// All rights reserved.

#include <tlrCore/File.h>

#include <tlrCore/String.h>

extern "C"
{
#include <fseq.h>
}

#if defined(_WINDOWS)
#include <direct.h>
#else
#include <unistd.h>
#endif

namespace tlr
{
    namespace file
    {
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

        bool changeDir(const std::string& path)
        {
            bool out = false;
#if defined(_WINDOWS)
            out = _chdir(path.c_str()) == 0;
#else
            out = chdir(path.c_str()) == 0;
#endif
            return out;
        }
    }
}