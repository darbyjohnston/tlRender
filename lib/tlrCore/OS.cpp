// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021 Darby Johnston
// All rights reserved.

#include <tlrCore/OS.h>

#include <tlrCore/Memory.h>
#include <tlrCore/String.h>

#include <cstdlib>

namespace tlr
{
    namespace os
    {
        size_t getRAMSizeGB()
        {
            auto d = std::lldiv(getRAMSize(), memory::gigabyte);
            return d.quot + (d.rem ? 1 : 0);
        }

        char getEnvListSeparator(EnvListSeparator value)
        {
            return EnvListSeparator::Unix == value ? ':' : ';';
        }

        bool getIntEnv(const std::string& name, int& out)
        {
            std::string value;
            if (getEnv(name, value))
            {
                out = !value.empty() ? std::stoi(value) : 0;
                return true;
            }
            return false;
        }

        bool getStringListEnv(const std::string& name, std::vector<std::string>& out)
        {
            std::string value;
            if (getEnv(name, value))
            {
                out = string::split(value, getEnvListSeparator());
                return true;
            }
            return false;
        }
    }
}
