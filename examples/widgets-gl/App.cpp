// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2023 Darby Johnston
// All rights reserved.

#include "App.h"

#include "MainWindow.h"

#include <tlUI/EventLoop.h>

namespace tl
{
    namespace examples
    {
        namespace widgets_gl
        {
            void App::_init(
                int argc,
                char* argv[],
                const std::shared_ptr<system::Context>& context)
            {
                IApp::_init(
                    argc,
                    argv,
                    context,
                    "widgets-gl",
                    "Example GLFW user interface application.");
                if (_exit != 0)
                {
                    return;
                }

                _mainWindow = MainWindow::create(_context);
                getEventLoop()->addWidget(_mainWindow);
            }

            App::App()
            {}

            App::~App()
            {}

            std::shared_ptr<App> App::create(
                int argc,
                char* argv[],
                const std::shared_ptr<system::Context>& context)
            {
                auto out = std::shared_ptr<App>(new App);
                out->_init(argc, argv, context);
                return out;
            }
        }
    }
}
