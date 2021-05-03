// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021 Darby Johnston
// All rights reserved.

#pragma once

#include <tlrRender/Util.h>
#include <tlrRender/Error.h>

#include <memory>
#include <sstream>
#include <string>
#include <vector>

namespace tlr
{
    namespace app
    {
        //! Base class for command line options.
        class ICmdLineOption : public std::enable_shared_from_this<ICmdLineOption>
        {
            TLR_NON_COPYABLE(ICmdLineOption);

        protected:
            ICmdLineOption(
                const std::vector<std::string>& names,
                const std::string& help,
                const std::string& argsHelp);

        public:
            virtual ~ICmdLineOption() = 0;

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

        //! Command line flag option.
        class CmdLineFlagOption : public ICmdLineOption
        {
        protected:
            CmdLineFlagOption(
                bool& value,
                const std::vector<std::string>& names,
                const std::string& help,
                const std::string& argsHelp);

        public:
            static std::shared_ptr<CmdLineFlagOption> create(
                bool& value,
                const std::vector<std::string>& names,
                const std::string& help,
                const std::string& argsHelp = std::string());

            void parse(std::vector<std::string>& args) override;

        private:
            bool& _value;
        };

        //! Command line value option.
        template<typename T>
        class CmdLineValueOption : public ICmdLineOption
        {
        protected:
            CmdLineValueOption(
                T& value,
                const std::vector<std::string>& names,
                const std::string& help,
                const std::string& argsHelp);

        public:
            static std::shared_ptr<CmdLineValueOption<T> > create(
                T& value,
                const std::vector<std::string>& names,
                const std::string& help,
                const std::string& argsHelp = std::string());

            void parse(std::vector<std::string>& args) override;

        private:
            T& _value;
        };

        //! Base class for command line arguments.
        class ICmdLineArg : public std::enable_shared_from_this<ICmdLineArg>
        {
        protected:
            ICmdLineArg(
                const std::string& name,
                const std::string& help,
                bool optional);

        public:
            virtual ~ICmdLineArg() = 0;

            //! Parse the argument.
            virtual void parse(std::vector<std::string>& args) = 0;

            //! Get the argument names.
            const std::string& getName() const;

            //! Get the help.
            const std::string& getHelp() const;

            //! Get whether this argument is optional.
            bool isOptional() const;

        protected:
            std::string _name;
            std::string _help;
            bool _optional = false;
        };

        //! Command line value argument.
        template<typename T>
        class CmdLineValueArg : public ICmdLineArg
        {
        protected:
            CmdLineValueArg(
                T& value,
                const std::string& name,
                const std::string& help,
                bool optional);

        public:
            static std::shared_ptr<CmdLineValueArg<T> > create(
                T& value,
                const std::string& name,
                const std::string& help,
                bool optional = false);

            void parse(std::vector<std::string>& args) override;

        private:
            T& _value;
        };
    }
}

#include <tlrApp/CmdLineInline.h>
