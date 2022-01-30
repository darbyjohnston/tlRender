// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2022 Darby Johnston
// All rights reserved.

#include <tlrCore/Matrix.h>

#include <tlrCore/Error.h>
#include <tlrCore/String.h>

#include <sstream>

using namespace tlr::core;

namespace tlr
{
    namespace math
    {
        std::ostream& operator << (std::ostream& os, const Matrix3x3f& value)
        {
            std::vector<std::string> s;
            for (size_t i = 0; i < 9; ++i)
            {
                std::stringstream ss;
                ss << value.e[i];
                s.push_back(ss.str());
            }
            os << string::join(s, ',');
            return os;
        }

        std::ostream& operator << (std::ostream& os, const Matrix4x4f& value)
        {
            std::vector<std::string> s;
            for (size_t i = 0; i < 16; ++i)
            {
                std::stringstream ss;
                ss << value.e[i];
                s.push_back(ss.str());
            }
            os << string::join(s, ',');
            return os;
        }

        std::istream& operator >> (std::istream& is, Matrix3x3f& value)
        {
            std::string s;
            is >> s;
            auto split = tlr::string::split(s, ',');
            if (split.size() != 9)
            {
                throw ParseError();
            }
            for (size_t i = 0; i < 9; ++i)
            {
                std::stringstream ss(split[i]);
                ss >> value.e[i];
            }
            return is;
        }

        std::istream& operator >> (std::istream& is, Matrix4x4f& value)
        {
            std::string s;
            is >> s;
            auto split = tlr::string::split(s, ',');
            if (split.size() != 16)
            {
                throw ParseError();
            }
            for (size_t i = 0; i < 16; ++i)
            {
                std::stringstream ss(split[i]);
                ss >> value.e[i];
            }
            return is;
        }
    }
}