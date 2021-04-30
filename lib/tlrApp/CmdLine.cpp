// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021 Darby Johnston
// All rights reserved.

#include <tlrApp/CmdLine.h>

namespace tlr
{
    namespace app
    {
        ICmdLineOption::ICmdLineOption(
            const std::vector<std::string>& names,
            const std::string& help,
            const std::string& argsHelp) :
            _names(names),
            _help(help),
            _argsHelp(argsHelp)
        {}

        ICmdLineOption::~ICmdLineOption()
        {}

        CmdLineFlagOption::CmdLineFlagOption(
            bool& value,
            const std::vector<std::string>&names,
            const std::string & help,
            const std::string & argsHelp) :
            ICmdLineOption(names, help, argsHelp),
            _value(value)
        {}

        std::shared_ptr<CmdLineFlagOption> CmdLineFlagOption::create(
            bool& value,
            const std::vector<std::string>& names,
            const std::string& help,
            const std::string& argsHelp)
        {
            return std::shared_ptr<CmdLineFlagOption>(new CmdLineFlagOption(value, names, help, argsHelp));
        }

        void CmdLineFlagOption::parse(std::vector<std::string>&args)
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
