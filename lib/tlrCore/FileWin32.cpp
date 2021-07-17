// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021 Darby Johnston
// All rights reserved.

#include <tlrCore/File.h>

#include <tlrCore/String.h>

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif // WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <combaseapi.h>

#define _STAT     struct _stati64
#define _STAT_FNC _wstati64

namespace tlr
{
    namespace file
    {
        bool exists(const std::string& fileName)
        {
            _STAT info;
            memset(&info, 0, sizeof(_STAT));
            return 0 == _STAT_FNC(string::toWide(fileName).c_str(), &info);
        }
        
        std::string getTemp()
        {
            std::string out;
            WCHAR buf[MAX_PATH];
            DWORD r = GetTempPathW(MAX_PATH, buf);
            if (r && r < MAX_PATH)
            {
                out = string::fromWide(buf);
            }
            return out;
        }

        std::string createTempDir()
        {
            std::string out;

            // Get the temporary directory.
            char path[MAX_PATH];
            DWORD r = GetTempPath(MAX_PATH, path);
            if (r)
            {
                out = std::string(path);

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
    }
}
