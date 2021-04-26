// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021 Darby Johnston
// All rights reserved.

#include <algorithm>

namespace tlr
{
    namespace app
    {
        inline const std::vector<std::string>& IOption::getNames() const
        {
            return _names;
        }

        inline const std::string& IOption::getName() const
        {
            return _name;
        }

        inline const std::string& IOption::getHelp() const
        {
            return _help;
        }

        inline const std::string& IOption::getArgsHelp() const
        {
            return _argsHelp;
        }

        template<typename T>
        inline ValueOption<T>::ValueOption(
            T& value,
            const std::vector<std::string>& names,
            const std::string& help,
            const std::string& argsHelp) :
            IOption(names, help, argsHelp),
            _value(value)
        {}

        template<typename T>
        inline void ValueOption<T>::parse(std::vector<std::string>& args)
        {
            for (const auto& name : _names)
            {
                _name = name;
                auto i = std::find(args.begin(), args.end(), name);
                if (i != args.end())
                {
                    i = args.erase(i);
                    if (i != args.end())
                    {
                        std::stringstream ss(*i);
                        ss >> _value;
                        i = args.erase(i);
                    }
                    else
                    {
                        throw ParseError();
                    }
                }
            }
        }
    }
}
