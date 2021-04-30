// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021 Darby Johnston
// All rights reserved.

#include <algorithm>

namespace tlr
{
    namespace app
    {
        inline const std::vector<std::string>& ICmdLineOption::getNames() const
        {
            return _names;
        }

        inline const std::string& ICmdLineOption::getName() const
        {
            return _name;
        }

        inline const std::string& ICmdLineOption::getHelp() const
        {
            return _help;
        }

        inline const std::string& ICmdLineOption::getArgsHelp() const
        {
            return _argsHelp;
        }

        template<typename T>
        inline CmdLineValueOption<T>::CmdLineValueOption(
            T& value,
            const std::vector<std::string>& names,
            const std::string& help,
            const std::string& argsHelp) :
            ICmdLineOption(names, help, argsHelp),
            _value(value)
        {}
        
        template<typename T>
        inline std::shared_ptr<CmdLineValueOption<T> > CmdLineValueOption<T>::create(
            T& value,
            const std::vector<std::string>& names,
            const std::string& help,
            const std::string& argsHelp)
        {
            return std::shared_ptr<CmdLineValueOption<T> >(new CmdLineValueOption<T>(value, names, help, argsHelp));
        }

        template<typename T>
        inline void CmdLineValueOption<T>::parse(std::vector<std::string>& args)
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

        inline ICmdLineArg::ICmdLineArg(
            const std::string& name,
            const std::string& help) :
            _name(name),
            _help(help)
        {}

        inline ICmdLineArg::~ICmdLineArg()
        {}

        inline const std::string& ICmdLineArg::getName() const
        {
            return _name;
        }

        inline const std::string& ICmdLineArg::getHelp() const
        {
            return _help;
        }

        template<typename T>
        inline CmdLineValueArg<T>::CmdLineValueArg(
            T& value,
            const std::string& name,
            const std::string& help) :
            ICmdLineArg(name, help),
            _value(value)
        {}

        template<typename T>
        inline std::shared_ptr<CmdLineValueArg<T> > CmdLineValueArg<T>::create(
            T& value,
            const std::string& name,
            const std::string& help)
        {
            return std::shared_ptr<CmdLineValueArg<T> >(new CmdLineValueArg<T>(value, name, help));
        }

        template<typename T>
        inline void CmdLineValueArg<T>::parse(std::vector<std::string>& args)
        {
            auto i = args.begin();
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
