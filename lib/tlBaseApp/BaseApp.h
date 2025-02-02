// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2025 Darby Johnston
// All rights reserved.

#pragma once

#include <tlBaseApp/CmdLine.h>

#include <dtk/core/Context.h>

#if defined(_WINDOWS)
#define TLRENDER_MAIN() \
    int wmain(int argc, wchar_t* argv[])
#else // _WINDOWS
#define TLRENDER_MAIN() \
    int main(int argc, char* argv[])
#endif // _WINDOWS

namespace tl
{
    //! Base application
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

        //! Convert command line arguments.
        std::vector<std::string> convert(int argc, char* argv[]);

        //! Convert command line arguments.
        std::vector<std::string> convert(int argc, wchar_t* argv[]);

        //! Base class for applications.
        class BaseApp : public std::enable_shared_from_this<BaseApp>
        {
            TLRENDER_NON_COPYABLE(BaseApp);

        protected:
            void _init(
                const std::shared_ptr<dtk::Context>&,
                const std::vector<std::string>&,
                const std::string& cmdLineName,
                const std::string& cmdLineSummary,
                const std::vector<std::shared_ptr<ICmdLineArg> >& = {},
                const std::vector<std::shared_ptr<ICmdLineOption> >& = {});

            BaseApp();

        public:
            virtual ~BaseApp() = 0;

            //! Get the context.
            const std::shared_ptr<dtk::Context>& getContext() const;

            //! Get the exit code.
            int getExit() const;

        protected:
            const std::string& _getCmdLineName() const;

            void _log(const std::string&, dtk::LogType = dtk::LogType::Message);

            void _print(const std::string&);
            void _printNewline();
            void _printError(const std::string&);

            std::shared_ptr<dtk::Context> _context;
            Options _options;
            int _exit = 0;

        private:
            int _parseCmdLine();
            void _printCmdLineHelp();

            TLRENDER_PRIVATE();
        };
    }
}
