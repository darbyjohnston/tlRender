// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2022 Darby Johnston
// All rights reserved.

#include <tlrCore/Vector.h>

#include <tlrCore/Error.h>
#include <tlrCore/String.h>

#include <sstream>

using namespace tlr::core;

namespace tlr
{
    namespace math
    {
        std::ostream& operator << (std::ostream& os, const Vector2i& value)
        {
            os << value.x << "," << value.y;
            return os;
        }

        std::ostream& operator << (std::ostream& os, const Vector2f& value)
        {
            os << value.x << "," << value.y;
            return os;
        }

        std::ostream& operator << (std::ostream& os, const Vector3f& value)
        {
            os << value.x << "," << value.y << "," << value.z;
            return os;
        }

        std::ostream& operator << (std::ostream& os, const Vector4f& value)
        {
            os << value.x << "," << value.y << "," << value.z << "," << value.w;
            return os;
        }

        std::istream& operator >> (std::istream& is, Vector2i& value)
        {
            std::string s;
            is >> s;
            auto split = tlr::string::split(s, ',');
            if (split.size() != 2)
            {
                throw ParseError();
            }
            {
                std::stringstream ss(split[0]);
                ss >> value.x;
            }
            {
                std::stringstream ss(split[1]);
                ss >> value.y;
            }
            return is;
        }

        std::istream& operator >> (std::istream& is, Vector2f& value)
        {
            std::string s;
            is >> s;
            auto split = tlr::string::split(s, ',');
            if (split.size() != 2)
            {
                throw ParseError();
            }
            {
                std::stringstream ss(split[0]);
                ss >> value.x;
            }
            {
                std::stringstream ss(split[1]);
                ss >> value.y;
            }
            return is;
        }

        std::istream& operator >> (std::istream& is, Vector3f& value)
        {
            std::string s;
            is >> s;
            auto split = tlr::string::split(s, ',');
            if (split.size() != 3)
            {
                throw ParseError();
            }
            {
                std::stringstream ss(split[0]);
                ss >> value.x;
            }
            {
                std::stringstream ss(split[1]);
                ss >> value.y;
            }
            {
                std::stringstream ss(split[2]);
                ss >> value.z;
            }
            return is;
        }

        std::istream& operator >> (std::istream& is, Vector4f& value)
        {
            std::string s;
            is >> s;
            auto split = tlr::string::split(s, ',');
            if (split.size() != 4)
            {
                throw ParseError();
            }
            {
                std::stringstream ss(split[0]);
                ss >> value.x;
            }
            {
                std::stringstream ss(split[1]);
                ss >> value.y;
            }
            {
                std::stringstream ss(split[2]);
                ss >> value.z;
            }
            {
                std::stringstream ss(split[3]);
                ss >> value.w;
            }
            return is;
        }
    }
}