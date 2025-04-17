// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2024 Darby Johnston
// All rights reserved.

#include "WindowActions.h"

#include "App.h"
#include "MainWindow.h"

namespace tl
{
    namespace examples
    {
        namespace player
        {
            void WindowActions::_init(
                const std::shared_ptr<dtk::Context>& context,
                const std::shared_ptr<App>& app,
                const std::shared_ptr<MainWindow>& mainWindow)
            {
                auto mainWindowWeak = std::weak_ptr<MainWindow>(mainWindow);
                _actions["FullScreen"] = dtk::Action::create(
                    "FullScreen",
                    "WindowFullScreen",
                    dtk::Key::F,
                    static_cast<int>(dtk::commandKeyModifier),
                    [mainWindowWeak](bool value)
                    {
                        if (auto mainWindow = mainWindowWeak.lock())
                        {
                            mainWindow->setFullScreen(value);
                        }
                    });
                _actions["FullScreen"]->setTooltip("Toggle the window full screen mode.");
            }

            WindowActions::~WindowActions()
            {}

            std::shared_ptr<WindowActions> WindowActions::create(
                const std::shared_ptr<dtk::Context>& context,
                const std::shared_ptr<App>& app,
                const std::shared_ptr<MainWindow>& mainWindow)
            {
                auto out = std::shared_ptr<WindowActions>(new WindowActions);
                out->_init(context, app, mainWindow);
                return out;
            }

            const std::map<std::string, std::shared_ptr<dtk::Action> >& WindowActions::getActions() const
            {
                return _actions;
            }
        }
    }
}
