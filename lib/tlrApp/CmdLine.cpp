// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021 Darby Johnston
// All rights reserved.

#include <tlrApp/CmdLine.h>

namespace tlr
{
    namespace app
    {
        IOption::IOption(
            const std::vector<std::string>& names,
            const std::string& help,
            const std::string& argsHelp) :
            _names(names),
            _help(help),
            _argsHelp(argsHelp)
        {}

        IOption::~IOption()
        {}

        FlagOption::FlagOption(
            bool& value,
            const std::vector<std::string>&names,
            const std::string & help,
            const std::string & argsHelp) :
            IOption(names, help, argsHelp),
            _value(value)
        {}

        void FlagOption::parse(std::vector<std::string>&args)
        {
            for (const auto& name : _names)
            {
                _name = name;
                auto i = std::find(args.begin(), args.end(), name);
                if (i != args.end())
                {
                    _value = true;
                    i = args.erase(i);
                }
            }
        }
    }
}
