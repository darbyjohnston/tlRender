// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021 Darby Johnston
// All rights reserved.

#include <tlCore/File.h>

#include <cstring>
#include <vector>

#include <sys/stat.h>
#include <stdlib.h>
#include <unistd.h>

#if defined(__APPLE__)
#define _STAT struct ::stat
#define _STAT_FNC    ::stat
#else // __APPLE__
#define _STAT struct ::stat64
#define _STAT_FNC    ::stat64
#endif // __APPLE__

namespace tl
{
	namespace file
	{
		bool exists(const std::string& fileName)
		{
			_STAT info;
			memset(&info, 0, sizeof(_STAT));
			return 0 == _STAT_FNC(fileName.c_str(), &info);
		}

        bool mkdir(const std::string& fileName)
        {
            return 0 == ::mkdir(fileName.c_str(), S_IRWXU | S_IRWXG);
        }

        bool rmdir(const std::string& fileName)
        {
            return 0 == ::rmdir(fileName.c_str());
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
					if (exists(path))
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
		    std::string out;
			const std::string path = getTemp() + "/XXXXXX";
			const size_t size = path.size();
			std::vector<char> buf(size + 1);
			std::memcpy(buf.data(), path.c_str(), size);
			buf[size] = 0;
			if (char* s = mkdtemp(buf.data()))
			{
			    out = s;
			}
			return out;
		}
	}
}
