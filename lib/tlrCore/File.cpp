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
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif // WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <combaseapi.h>
#else // _WINDOWS
#include <sys/stat.h>
#include <stdlib.h>
#include <unistd.h>
#endif // _WINDOWS

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

#if defined(_WINDOWS)
        std::string createTempDir()
        {
            std::string out;

            // Get the temporary directory.
            char path[MAX_PATH];
            DWORD r = GetTempPath(MAX_PATH, path);
            if (r)
            {
                out = std::string(path);

                // Replace path separators.
                for (size_t i = 0; i < out.size(); ++i)
                {
                    if ('\\' == out[i])
                    {
                        out[i] = '/';
                    }
                }

                // Create a unique name from a GUID.
                GUID guid;
                CoCreateGuid(&guid);
                const uint8_t* guidP = reinterpret_cast<const uint8_t*>(&guid);
                for (int i = 0; i < 16; ++i)
                {
                    char buf[3] = "";
                    sprintf_s(buf, 3, "%02x", guidP[i]);
                    out += buf;
                }

                // Create a unique directory.
                CreateDirectory(out.c_str(), NULL);
            }

            return out;
        }
#else
        std::string createTempDir()
        {
            // Find the temporary directory.
            std::string path;
            char* env = nullptr;
            if ((env = getenv("TEMP"))) path = env;
            else if ((env = getenv("TMP"))) path = env;
            else if ((env = getenv("TMPDIR"))) path = env;
            else
            {
                for (const auto& i : { "/tmp", "/var/tmp", "/usr/tmp" })
                {
                    struct stat buffer;
                    if (0 == stat(i, &buffer))
                    {
                        path = i;
                        break;
                    }
                }
            }

            // Create a unique directory.
            path = path + "/XXXXXX";
            const size_t size = path.size();
            std::vector<char> buf(size + 1);
            memcpy(buf.data(), path.c_str(), size);
            buf[size] = 0;
            return mkdtemp(buf.data());
        }
#endif
    }
}