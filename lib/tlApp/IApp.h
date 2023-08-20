// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2023 Darby Johnston
// All rights reserved.

#pragma once

#include <tlApp/CmdLine.h>

#include <tlCore/Context.h>

namespace tl
{
    //! General application functionality.
    namespace app
    {
        class ICmdLineArg;
        class ICmdLineOption;

        //! Application options.
        struct Options
        {
            bool log = false;
            bool help = false;
        };

        //! Base class for applications.
        class IApp : public std::enable_shared_from_this<IApp>
        {
            TLRENDER_NON_COPYABLE(IApp);

        protected:
            void _init(
                int argc,
                char* argv[],
                const std::shared_ptr<system::Context>&,
                const std::string& cmdLineName,
                const std::string& cmdLineSummary,
                const std::vector<std::shared_ptr<ICmdLineArg> >& = {},
                const std::vector<std::shared_ptr<ICmdLineOption> >& = {});

            IApp();

        public:
            virtual ~IApp() = 0;

            //! Get the context.
            const std::shared_ptr<system::Context>& getContext() const;

            //! Get the exit code.
            int getExit() const;

        protected:
            void _log(const std::string&, log::Type = log::Type::Message);

            void _print(const std::string&);
            void _printNewline();
            void _printError(const std::string&);

            std::shared_ptr<system::Context> _context;
            Options _options;
            int _exit = 0;

        private:
            int _parseCmdLine();
            void _printCmdLineHelp();

            TLRENDER_PRIVATE();
        };
    }
}
