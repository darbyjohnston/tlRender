// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2024 Darby Johnston
// All rights reserved.

#include "ViewActions.h"

#include "App.h"
#include "MainWindow.h"

#include <tlTimelineUI/Viewport.h>

namespace tl
{
    namespace examples
    {
        namespace player
        {
            void ViewActions::_init(
                const std::shared_ptr<dtk::Context>& context,
                const std::shared_ptr<App>& app,
                const std::shared_ptr<MainWindow>& mainWindow)
            {
                auto mainWindowWeak = std::weak_ptr<MainWindow>(mainWindow);
                _actions["Frame"] = dtk::Action::create(
                    "Frame",
                    "ViewFrame",
                    dtk::Key::Backspace,
                    0,
                    [mainWindowWeak](bool value)
                    {
                        if (auto mainWindow = mainWindowWeak.lock())
                        {
                            mainWindow->getViewport()->setFrameView(value);
                        }
                    });
                _actions["Frame"]->setTooltip("Toggle whether the view is automatically framed.");

                _frameObserver = dtk::ValueObserver<bool>::create(
                    mainWindow->getViewport()->observeFrameView(),
                    [this](bool value)
                    {
                        _actions["Frame"]->setChecked(value);
                    });
            }

            ViewActions::~ViewActions()
            {}

            std::shared_ptr<ViewActions> ViewActions::create(
                const std::shared_ptr<dtk::Context>& context,
                const std::shared_ptr<App>& app,
                const std::shared_ptr<MainWindow>& mainWindow)
            {
                auto out = std::shared_ptr<ViewActions>(new ViewActions);
                out->_init(context, app, mainWindow);
                return out;
            }

            const std::map<std::string, std::shared_ptr<dtk::Action> >& ViewActions::getActions() const
            {
                return _actions;
            }
        }
    }
}
