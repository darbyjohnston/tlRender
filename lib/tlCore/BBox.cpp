// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2023 Darby Johnston
// All rights reserved.

#include <tlCore/BBox.h>

#include <tlCore/Error.h>
#include <tlCore/String.h>

#include <sstream>

namespace tl
{
    namespace math
    {
        BBox2i operator * (const BBox2i& a, float b)
        {
            BBox2i out;
            out.min = a.min * b;
            out.max = a.max * b;
            return out;
        }

        BBox2f operator * (const BBox2f& a, float b)
        {
            BBox2f out;
            out.min = a.min * b;
            out.max = a.max * b;
            return out;
        }

        void to_json(nlohmann::json& json, const BBox2i& value)
        {
            json = { value.min, value.max };
        }

        void to_json(nlohmann::json& json, const BBox2f& value)
        {
            json = { value.min, value.max };
        }

        void from_json(const nlohmann::json& json, BBox2i& value)
        {
            json.at(0).get_to(value.min);
            json.at(1).get_to(value.max);
        }

        void from_json(const nlohmann::json& json, BBox2f& value)
        {
            json.at(0).get_to(value.min);
            json.at(1).get_to(value.max);
        }

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
            auto split = string::split(s, '-');
            if (split.size() != 2)
            {
                throw error::ParseError();
            }
            {
                std::stringstream ss(split[0]);
                ss >> value.min;
            }
            {
                std::stringstream ss(split[1]);
                ss >> value.max;
            }
            return is;
        }

        std::istream& operator >> (std::istream& is, BBox2f& value)
        {
            std::string s;
            is >> s;
            auto split = string::split(s, '-');
            if (split.size() != 2)
            {
                throw error::ParseError();
            }
            {
                std::stringstream ss(split[0]);
                ss >> value.min;
            }
            {
                std::stringstream ss(split[1]);
                ss >> value.max;
            }
            return is;
        }
    }
}
