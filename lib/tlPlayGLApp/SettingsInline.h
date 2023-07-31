// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2023 Darby Johnston
// All rights reserved.

namespace tl
{
    namespace play_gl
    {
        template<typename T>
        inline void Settings::getValue(const std::string& key, T& out) const
        {
            if (_values.contains(key))
            {
                try
                {
                    //from_json(_values.at(key), out);
                    out = _values.at(key);
                }
                catch (const std::exception&)
                {}
            }
        }

        /*template<>
        inline void Settings::getValue(const std::string& key, bool& out) const
        {
            if (_values.contains(key))
            {
                try
                {
                    out = _values.at(key);
                }
                catch (const std::exception&)
                {}
            }
        }

        template<>
        inline void Settings::getValue(const std::string& key, int& out) const
        {
            if (_values.contains(key))
            {
                try
                {
                    out = _values.at(key);
                }
                catch (const std::exception&)
                {}
            }
        }

        template<>
        inline void Settings::getValue(const std::string& key, float& out) const
        {
            if (_values.contains(key))
            {
                try
                {
                    out = _values.at(key);
                }
                catch (const std::exception&)
                {}
            }
        }

        template<>
        inline void Settings::getValue(const std::string& key, double& out) const
        {
            if (_values.contains(key))
            {
                try
                {
                    out = _values.at(key);
                }
                catch (const std::exception&)
                {}
            }
        }

        template<>
        inline void Settings::getValue(const std::string& key, size_t& out) const
        {
            if (_values.contains(key))
            {
                try
                {
                    out = _values.at(key);
                }
                catch (const std::exception&)
                {}
            }
        }

        template<>
        inline void Settings::getValue(const std::string& key, std::string& out) const
        {
            if (_values.contains(key))
            {
                try
                {
                    out = _values.at(key);
                }
                catch (const std::exception&)
                {}
            }
        }

        template<>
        inline void Settings::getValue(const std::string& key, std::vector<std::string>& out) const
        {
            if (_values.contains(key))
            {
                try
                {
                    out = _values.at(key);
                }
                catch (const std::exception&)
                {}
            }
        }*/

        template<typename T>
        inline void Settings::setDefaultValue(const std::string& key, T in)
        {
            //nlohmann::json json;
            //to_json(json, in);
            nlohmann::json json = in;
            _defaultValues[key] = json;
            const auto i = _values.find(key);
            if (i == _values.end())
            {
                _values[key] = json;
                _observer->setAlways(key);
            }
        }

        /*template<>
        inline void Settings::setDefaultValue(const std::string& key, bool in)
        {
            nlohmann::json json = in;
            _defaultValues[key] = json;
            const auto i = _values.find(key);
            if (i == _values.end())
            {
                _values[key] = json;
                _observer->setAlways(key);
            }
        }

        template<>
        inline void Settings::setDefaultValue(const std::string& key, int in)
        {
            nlohmann::json json = in;
            _defaultValues[key] = json;
            const auto i = _values.find(key);
            if (i == _values.end())
            {
                _values[key] = json;
                _observer->setAlways(key);
            }
        }

        template<>
        inline void Settings::setDefaultValue(const std::string& key, float in)
        {
            nlohmann::json json = in;
            _defaultValues[key] = json;
            const auto i = _values.find(key);
            if (i == _values.end())
            {
                _values[key] = json;
                _observer->setAlways(key);
            }
        }

        template<>
        inline void Settings::setDefaultValue(const std::string& key, double in)
        {
            nlohmann::json json = in;
            _defaultValues[key] = json;
            const auto i = _values.find(key);
            if (i == _values.end())
            {
                _values[key] = json;
                _observer->setAlways(key);
            }
        }

        template<>
        inline void Settings::setDefaultValue(const std::string& key, size_t in)
        {
            nlohmann::json json = in;
            _defaultValues[key] = json;
            const auto i = _values.find(key);
            if (i == _values.end())
            {
                _values[key] = json;
                _observer->setAlways(key);
            }
        }

        template<>
        inline void Settings::setDefaultValue(const std::string& key, std::string in)
        {
            nlohmann::json json = in;
            _defaultValues[key] = json;
            const auto i = _values.find(key);
            if (i == _values.end())
            {
                _values[key] = json;
                _observer->setAlways(key);
            }
        }

        template<>
        inline void Settings::setDefaultValue(const std::string& key, std::vector<std::string> in)
        {
            nlohmann::json json = in;
            _defaultValues[key] = json;
            const auto i = _values.find(key);
            if (i == _values.end())
            {
                _values[key] = json;
                _observer->setAlways(key);
            }
        }*/

        template<typename T>
        inline void Settings::setValue(const std::string& key, T in)
        {
            //nlohmann::json json;
            //to_json(json, in);
            nlohmann::json json = in;
            const auto i = _values.find(key);
            if (i == _values.end() || json != _values[key])
            {
                _values[key] = json;
                _observer->setAlways(key);
            }
        }

        /*template<>
        inline void Settings::setValue(const std::string& key, bool in)
        {
            nlohmann::json json = in;
            const auto i = _values.find(key);
            if (i == _values.end() || json != _values[key])
            {
                _values[key] = json;
                _observer->setAlways(key);
            }
        }

        template<>
        inline void Settings::setValue(const std::string& key, int in)
        {
            nlohmann::json json = in;
            const auto i = _values.find(key);
            if (i == _values.end() || json != _values[key])
            {
                _values[key] = json;
                _observer->setAlways(key);
            }
        }

        template<>
        inline void Settings::setValue(const std::string& key, float in)
        {
            nlohmann::json json = in;
            const auto i = _values.find(key);
            if (i == _values.end() || json != _values[key])
            {
                _values[key] = json;
                _observer->setAlways(key);
            }
        }

        template<>
        inline void Settings::setValue(const std::string& key, double in)
        {
            nlohmann::json json = in;
            const auto i = _values.find(key);
            if (i == _values.end() || json != _values[key])
            {
                _values[key] = json;
                _observer->setAlways(key);
            }
        }

        template<>
        inline void Settings::setValue(const std::string& key, size_t in)
        {
            nlohmann::json json = in;
            const auto i = _values.find(key);
            if (i == _values.end() || json != _values[key])
            {
                _values[key] = json;
                _observer->setAlways(key);
            }
        }

        template<>
        inline void Settings::setValue(const std::string& key, std::string in)
        {
            nlohmann::json json = in;
            const auto i = _values.find(key);
            if (i == _values.end() || json != _values[key])
            {
                _values[key] = json;
                _observer->setAlways(key);
            }
        }

        template<>
        inline void Settings::setValue(const std::string& key, std::vector<std::string> in)
        {
            nlohmann::json json = in;
            const auto i = _values.find(key);
            if (i == _values.end() || json != _values[key])
            {
                _values[key] = json;
                _observer->setAlways(key);
            }
        }*/
    }
}
