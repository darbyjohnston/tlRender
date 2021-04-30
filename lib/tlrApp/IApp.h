// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021 Darby Johnston
// All rights reserved.

#pragma once

#include <tlrApp/CmdLine.h>

#include <tlrRender/FontSystem.h>
#include <tlrRender/Render.h>

#include <tlrAV/IO.h>

#include <memory>
#include <string>
#include <vector>

struct GLFWwindow;

namespace tlr
{
    //! Applications.
    namespace app
    {
        //! Application options.
        struct Options
        {
            size_t ioVideoQueueSize = 10;
            bool verbose = false;
            bool help = false;
        };

        //! Base class for applications.
        class IApp : public std::enable_shared_from_this<IApp>
        {
            TLR_NON_COPYABLE(IApp);

        protected:
            void _init(
                int argc,
                char* argv[],
                const std::string& cmdLineName,
                const std::string& cmdLineSummary,
                const std::vector<std::shared_ptr<ICmdLineArg> >& = {},
                const std::vector<std::shared_ptr<ICmdLineOption> >& = {});
            IApp();

        public:
            virtual ~IApp() = 0;

            //! Get the exit code.
            int getExit() const;

        protected:
            void _createWindow(const imaging::Size&);
            void _destroyWindow();

            void _print(const std::string&);
            void _printVerbose(const std::string&);
            void _printError(const std::string&);

            Options _options;

            int _exit = 0;

            std::shared_ptr<av::io::System> _ioSystem;

            GLFWwindow* _glfwWindow = nullptr;
            imaging::Size _windowSize;
            imaging::Size _frameBufferSize;
            math::Vector2f _contentScale;

            std::shared_ptr<render::FontSystem> _fontSystem;
            std::shared_ptr<render::Render> _render;

        private:
            int _parseCmdLine();
            void _printCmdLineHelp();

            std::vector<std::string> _cmdLine;
            std::string _cmdLineName;
            std::string _cmdLineSummary;
            std::vector<std::shared_ptr<ICmdLineArg> > _cmdLineArgs;
            std::vector<std::shared_ptr<ICmdLineOption> > _cmdLineOptions;
        };
    }
}
