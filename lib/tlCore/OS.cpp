// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2022 Darby Johnston
// All rights reserved.

#include <tlCore/OS.h>

#include <tlCore/String.h>

namespace tl
{
    namespace core
    {
        namespace os
        {
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
}
