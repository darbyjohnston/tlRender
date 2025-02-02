// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2024 Darby Johnston
// All rights reserved.

#include "App.h"

#include "MainWindow.h"

#include <tlUI/FileBrowser.h>

namespace tl
{
    namespace examples
    {
        namespace widgets
        {
            void App::_init(
                const std::shared_ptr<dtk::Context>& context,
                const std::vector<std::string>& argv)
            {
                ui_app::App::_init(
                    context,
                    argv,
                    "widgets",
                    "Example widgets application.");
                if (_exit != 0)
                {
                    return;
                }

                auto fileBrowserSystem = context->getSystem<ui::FileBrowserSystem>();
                fileBrowserSystem->setNativeFileDialog(false);

                auto mainWindow = MainWindow::create(_context);
                addWindow(mainWindow);
                mainWindow->show();
            }

            App::App()
            {}

            App::~App()
            {}

            std::shared_ptr<App> App::create(
                const std::shared_ptr<dtk::Context>& context,
                const std::vector<std::string>& argv)
            {
                auto out = std::shared_ptr<App>(new App);
                out->_init(context, argv);
                return out;
            }
        }
    }
}
