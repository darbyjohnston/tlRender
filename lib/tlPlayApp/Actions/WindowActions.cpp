// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2025 Darby Johnston
// All rights reserved.

#include <tlPlayApp/Actions/WindowActions.h>

#include <tlPlayApp/App.h>
#include <tlPlayApp/MainWindow.h>

namespace tl
{
    namespace play
    {
        void WindowActions::_init(
            const std::shared_ptr<dtk::Context>& context,
            const std::shared_ptr<App>& app,
            const std::shared_ptr<MainWindow>& mainWindow)
        {
            IActions::_init(context, app, "Window");

            auto appWeak = std::weak_ptr<App>(app);
            _actions["FullScreen"] = dtk::Action::create(
                "Full Screen",
                "WindowFullScreen",
                [appWeak](bool value)
                {
                    if (auto app = appWeak.lock())
                    {
                        app->getMainWindow()->setFullScreen(value);
                    }
                });

            _actions["FloatOnTop"] = dtk::Action::create(
                "Float On Top",
                [appWeak](bool value)
                {
                    if (auto app = appWeak.lock())
                    {
                        app->getMainWindow()->setFloatOnTop(value);
                    }
                });

            _actions["Secondary"] = dtk::Action::create(
                "Secondary",
                "WindowSecondary",
                [appWeak](bool value)
                {
                    if (auto app = appWeak.lock())
                    {
                        app->setSecondaryWindow(value);
                    }
                });

            _actions["FileToolBar"] = dtk::Action::create(
                "File Tool Bar",
                [appWeak](bool value)
                {
                    if (auto app = appWeak.lock())
                    {
                        auto options = app->getSettingsModel()->getWindow();
                        options.fileToolBar = value;
                        app->getSettingsModel()->setWindow(options);
                    }
                });

            _actions["CompareToolBar"] = dtk::Action::create(
                "Compare Tool Bar",
                [appWeak](bool value)
                {
                    if (auto app = appWeak.lock())
                    {
                        auto options = app->getSettingsModel()->getWindow();
                        options.compareToolBar = value;
                        app->getSettingsModel()->setWindow(options);
                    }
                });

            _actions["WindowToolBar"] = dtk::Action::create(
                "Window Tool Bar",
                [appWeak](bool value)
                {
                    if (auto app = appWeak.lock())
                    {
                        auto options = app->getSettingsModel()->getWindow();
                        options.windowToolBar = value;
                        app->getSettingsModel()->setWindow(options);
                    }
                });

            _actions["ViewToolBar"] = dtk::Action::create(
                "View Tool Bar",
                [appWeak](bool value)
                {
                    if (auto app = appWeak.lock())
                    {
                        auto options = app->getSettingsModel()->getWindow();
                        options.viewToolBar = value;
                        app->getSettingsModel()->setWindow(options);
                    }
                });

            _actions["ToolsToolBar"] = dtk::Action::create(
                "Tools Tool Bar",
                [appWeak](bool value)
                {
                    if (auto app = appWeak.lock())
                    {
                        auto options = app->getSettingsModel()->getWindow();
                        options.toolsToolBar = value;
                        app->getSettingsModel()->setWindow(options);
                    }
                });

            _actions["Timeline"] = dtk::Action::create(
                "Timeline",
                [appWeak](bool value)
                {
                    if (auto app = appWeak.lock())
                    {
                        auto options = app->getSettingsModel()->getWindow();
                        options.timeline = value;
                        app->getSettingsModel()->setWindow(options);
                    }
                });

            _actions["BottomToolBar"] = dtk::Action::create(
                "Bottom Tool Bar",
                [appWeak](bool value)
                {
                    if (auto app = appWeak.lock())
                    {
                        auto options = app->getSettingsModel()->getWindow();
                        options.bottomToolBar = value;
                        app->getSettingsModel()->setWindow(options);
                    }
                });

            _actions["StatusToolBar"] = dtk::Action::create(
                "Status Tool Bar",
                [appWeak](bool value)
                {
                    if (auto app = appWeak.lock())
                    {
                        auto options = app->getSettingsModel()->getWindow();
                        options.statusToolBar = value;
                        app->getSettingsModel()->setWindow(options);
                    }
                });

            _tooltips =
            {
                { "FullScreen", "Toggle the window full screen." },
                { "Secondary", "Toggle the secondary window." }
            };

            _keyShortcutsUpdate(app->getSettingsModel()->getKeyShortcuts());
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
    }
}
