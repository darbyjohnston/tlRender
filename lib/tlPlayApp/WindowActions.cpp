// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2025 Darby Johnston
// All rights reserved.

#include <tlPlayApp/WindowActions.h>

#include <tlPlayApp/App.h>
#include <tlPlayApp/MainWindow.h>

#include <dtk/core/Format.h>

namespace tl
{
    namespace play_app
    {
        struct WindowActions::Private
        {
            std::shared_ptr<play::SettingsModel> model;
            std::map<std::string, std::shared_ptr<dtk::Action> > actions;
        };

        void WindowActions::_init(
            const std::shared_ptr<dtk::Context>& context,
            const std::shared_ptr<App>& app,
            const std::shared_ptr<MainWindow>& mainWindow)
        {
            DTK_P();

            p.model = app->getSettingsModel();

            auto appWeak = std::weak_ptr<App>(app);
            p.actions["FullScreen"] = std::make_shared<dtk::Action>(
                "Full Screen",
                "WindowFullScreen",
                dtk::Key::U,
                0,
                [appWeak](bool value)
                {
                    if (auto app = appWeak.lock())
                    {
                        app->getMainWindow()->setFullScreen(value);
                    }
                });
            p.actions["FullScreen"]->toolTip = dtk::Format(
                "Toggle the window full screen\n"
                "\n"
                "Shortcut: {0}").
                arg(dtk::getShortcutLabel(
                    p.actions["FullScreen"]->shortcut,
                    p.actions["FullScreen"]->shortcutModifiers));

            p.actions["FloatOnTop"] = std::make_shared<dtk::Action>(
                "Float On Top",
                [appWeak](bool value)
                {
                    if (auto app = appWeak.lock())
                    {
                        app->getMainWindow()->setFloatOnTop(value);
                    }
                });

            p.actions["Secondary"] = std::make_shared<dtk::Action>(
                "Secondary",
                "WindowSecondary",
                dtk::Key::Y,
                0,
                [appWeak](bool value)
                {
                    if (auto app = appWeak.lock())
                    {
                        app->setSecondaryWindow(value);
                    }
                });
            p.actions["Secondary"]->toolTip = dtk::Format(
                "Toggle the secondary window\n"
                "\n"
                "Shortcut: {0}").
                arg(dtk::getShortcutLabel(
                    p.actions["Secondary"]->shortcut,
                    p.actions["Secondary"]->shortcutModifiers));

            p.actions["FileToolBar"] = std::make_shared<dtk::Action>(
                "File Tool Bar",
                [this](bool value)
                {
                    DTK_P();
                    auto options = p.model->getWindow();
                    options.fileToolBar = value;
                    p.model->setWindow(options);
                });

            p.actions["CompareToolBar"] = std::make_shared<dtk::Action>(
                "Compare Tool Bar",
                [this](bool value)
                {
                    DTK_P();
                    auto options = p.model->getWindow();
                    options.compareToolBar = value;
                    p.model->setWindow(options);
                });

            p.actions["WindowToolBar"] = std::make_shared<dtk::Action>(
                "Window Tool Bar",
                [this](bool value)
                {
                    DTK_P();
                    auto options = p.model->getWindow();
                    options.windowToolBar = value;
                    p.model->setWindow(options);
                });

            p.actions["ViewToolBar"] = std::make_shared<dtk::Action>(
                "View Tool Bar",
                [this](bool value)
                {
                    DTK_P();
                    auto options = p.model->getWindow();
                    options.viewToolBar = value;
                    p.model->setWindow(options);
                });

            p.actions["ToolsToolBar"] = std::make_shared<dtk::Action>(
                "Tools Tool Bar",
                [this](bool value)
                {
                    DTK_P();
                    auto options = p.model->getWindow();
                    options.toolsToolBar = value;
                    p.model->setWindow(options);
                });

            p.actions["Timeline"] = std::make_shared<dtk::Action>(
                "Timeline",
                [this](bool value)
                {
                    DTK_P();
                    auto options = p.model->getWindow();
                    options.timeline = value;
                    p.model->setWindow(options);
                });

            p.actions["BottomToolBar"] = std::make_shared<dtk::Action>(
                "Bottom Tool Bar",
                [this](bool value)
                {
                    DTK_P();
                    auto options = p.model->getWindow();
                    options.bottomToolBar = value;
                    p.model->setWindow(options);
                });

            p.actions["StatusToolBar"] = std::make_shared<dtk::Action>(
                "Status Tool Bar",
                [this](bool value)
                {
                    DTK_P();
                    auto options = p.model->getWindow();
                    options.statusToolBar = value;
                    p.model->setWindow(options);
                });
        }

        WindowActions::WindowActions() :
            _p(new Private)
        {}

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
            return _p->actions;
        }
    }
}
