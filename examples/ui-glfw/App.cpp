// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2023 Darby Johnston
// All rights reserved.

#include "App.h"

#include "MainWindow.h"

namespace tl
{
    namespace examples
    {
        namespace ui_glfw
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
                    "ui-glfw",
                    "Example GLFW user interface application.");
                if (_exit != 0)
                {
                    return;
                }

                _mainWindow = MainWindow::create(_context);
                addWindow(_mainWindow);
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
