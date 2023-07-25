// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2023 Darby Johnston
// All rights reserved.

namespace tl
{
    namespace play_gl
    {
        template<typename T>
        T Settings::getData(const std::string& key) const
        {
            T out;
            std::stringstream ss(getData(key));
            ss >> out;
            return out;
        }

        template<typename T>
        void Settings::setData(const std::string& key, T value)
        {
            std::stringstream ss;
            ss << value;
            setData(key, ss.str());
        }
    }
}
