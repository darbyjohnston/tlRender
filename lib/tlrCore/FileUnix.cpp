// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021 Darby Johnston
// All rights reserved.

#include <tlrCore/File.h>

#include <cstring>

#include <sys/stat.h>
#include <stdlib.h>
#include <unistd.h>

namespace tlr
{
    namespace file
    {
        bool exists(const std::string& fileName)
        {
            struct stat64 info;
            memset(&info, 0, sizeof(struct stat64));
            return 0 == stat64(fileName.c_str(), &info);
        }
        
        std::string getTemp()
        {
            std::string out;
            char* env = nullptr;
            if ((env = getenv("TEMP")))
            {
                out = env;
            }
            else if ((env = getenv("TMP")))
            {
                out = env;
            }
            else if ((env = getenv("TMPDIR")))
            {
                out = env;
            }
            else
            {
                for (const auto& path : { "/tmp", "/var/tmp", "/usr/tmp" })
                {
                    if (exists(std::string(path)))
                    {
                        out = path;
                        break;
                    }
                }
            }
            return out;
        }

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
            std::memcpy(buf.data(), path.c_str(), size);
            buf[size] = 0;
            return mkdtemp(buf.data());
        }
    }
}
