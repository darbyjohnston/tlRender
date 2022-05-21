// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2022 Darby Johnston
// All rights reserved.

#include <tlCore/Color.h>

#include <tlCore/Error.h>
#include <tlCore/String.h>

#include <sstream>

namespace tl
{
    namespace imaging
    {
        std::ostream& operator << (std::ostream& os, const Color4f& value)
        {
            os << value.r << "," << value.g << "," << value.b << "," << value.a;
            return os;
        }

        std::istream& operator >> (std::istream& is, Color4f& value)
        {
            std::string s;
            is >> s;
            auto split = string::split(s, ',');
            if (split.size() != 4)
            {
                throw error::ParseError();
            }
            {
                std::stringstream ss(split[0]);
                ss >> value.r;
            }
            {
                std::stringstream ss(split[1]);
                ss >> value.g;
            }
            {
                std::stringstream ss(split[2]);
                ss >> value.b;
            }
            {
                std::stringstream ss(split[3]);
                ss >> value.a;
            }
            return is;
        }
    }
}
