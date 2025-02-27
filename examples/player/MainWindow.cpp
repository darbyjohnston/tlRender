// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2024 Darby Johnston
// All rights reserved.

#include "MainWindow.h"

#include <tlTimelineUI/Viewport.h>

#include <tlTimelineGL/Render.h>

namespace tl
{
    namespace examples
    {
        namespace player
        {
            void MainWindow::_init(
                const std::shared_ptr<dtk::Context>& context,
                const std::shared_ptr<dtk::App>& app)
            {
                dtk::MainWindow::_init(context, app, "player", dtk::Size2I(1280, 720));
            }

            MainWindow::~MainWindow()
            {}

            std::shared_ptr<MainWindow> MainWindow::create(
                const std::shared_ptr<dtk::Context>& context,
                const std::shared_ptr<dtk::App>& app)
            {
                auto out = std::shared_ptr<MainWindow>(new MainWindow);
                out->_init(context, app);
                return out;
            }

            std::shared_ptr<dtk::IRender> MainWindow::_createRender(const std::shared_ptr<dtk::Context>& context)
            {
                return timeline_gl::Render::create(context);
            }
        }
    }
}
