// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2023 Darby Johnston
// All rights reserved.

namespace tl
{
    namespace play_gl
    {
        template<typename T>
        T Settings::getValue(const std::string& key) const
        {
            T out;
            std::stringstream ss(getValue(key));
            ss >> out;
            return out;
        }

        template<typename T>
        void Settings::setValue(const std::string& key, T value)
        {
            std::stringstream ss;
            ss << value;
            setValue(key, ss.str());
        }
    }
}
