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
            T out = T();
            if (_values.contains(key))
            {
                from_json(_values.at(key), out);
            }
            return out;
        }

        template<typename T>
        void Settings::setValue(const std::string& key, T value)
        {
            nlohmann::json json;
            to_json(json, value);
            if (!_values.contains(key))
            {
                _defaultValues[key] = json;
            }
            if (json != _values[key])
            {
                _values[key] = json;
                _observer->setAlways(key);
            }
        }
    }
}
