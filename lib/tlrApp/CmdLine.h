// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021 Darby Johnston
// All rights reserved.

#pragma once

#include <tlrCore/Error.h>

#include <sstream>
#include <string>
#include <vector>

namespace tlr
{
    namespace app
    {
        //! Command Line Option Interface
        class IOption
        {
        public:
            IOption(
                const std::vector<std::string>& names,
                const std::string& help,
                const std::string& argsHelp = std::string());

            virtual ~IOption() = 0;

            //! Parse the option.
            virtual void parse(std::vector<std::string>& args) = 0;

            //! Get the option names.
            const std::vector<std::string>& getNames() const;

            //! Get the option name that was used.
            const std::string& getName() const;

            //! Get the help.
            const std::string& getHelp() const;

            //! Get the arguments help.
            const std::string& getArgsHelp() const;

        protected:
            std::vector<std::string> _names;
            std::string _name;
            std::string _help;
            std::string _argsHelp;
        };

        //! Command Line Flag Option
        class FlagOption : public IOption
        {
        public:
            FlagOption(
                bool& value,
                const std::vector<std::string>& names,
                const std::string& help,
                const std::string& argsHelp = std::string());

            void parse(std::vector<std::string>& args) override;

        private:
            bool& _value;
        };

        //! Command Line Value Option
        template<typename T>
        class ValueOption : public IOption
        {
        public:
            ValueOption(
                T& value,
                const std::vector<std::string>& names,
                const std::string& help,
                const std::string& argsHelp = std::string());

            void parse(std::vector<std::string>& args) override;

        private:
            T& _value;
        };
    }
}

#include <tlrApp/CmdLineInline.h>
