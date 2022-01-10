// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2022 Darby Johnston
// All rights reserved.

#include <tlrCore/BBox.h>

#include <tlrCore/Error.h>
#include <tlrCore/String.h>

#include <sstream>

using namespace tlr::core;

namespace tlr
{
    namespace math
    {
        std::ostream& operator << (std::ostream& os, const BBox2i& value)
        {
            os << value.min.x << "," << value.min.y << "-" << value.max.x << "," << value.max.y;
            return os;
        }

        std::ostream& operator << (std::ostream& os, const BBox2f& value)
        {
            os << value.min.x << "," << value.min.y << "-" << value.max.x << "," << value.max.y;
            return os;
        }

        std::istream& operator >> (std::istream& is, BBox2i& value)
        {
            std::string s;
            is >> s;
            auto split = tlr::string::split(s, '-');
            if (split.size() != 4)
            {
                throw ParseError();
            }
            {
                std::stringstream ss(split[0]);
                ss >> value.min.x;
            }
            {
                std::stringstream ss(split[1]);
                ss >> value.min.y;
            }
            {
                std::stringstream ss(split[2]);
                ss >> value.max.x;
            }
            {
                std::stringstream ss(split[3]);
                ss >> value.max.y;
            }
            return is;
        }

        std::istream& operator >> (std::istream& is, BBox2f& value)
        {
            std::string s;
            is >> s;
            auto split = tlr::string::split(s, '-');
            if (split.size() != 4)
            {
                throw ParseError();
            }
            {
                std::stringstream ss(split[0]);
                ss >> value.min.x;
            }
            {
                std::stringstream ss(split[1]);
                ss >> value.min.y;
            }
            {
                std::stringstream ss(split[2]);
                ss >> value.max.x;
            }
            {
                std::stringstream ss(split[3]);
                ss >> value.max.y;
            }
            return is;
        }
    }
}