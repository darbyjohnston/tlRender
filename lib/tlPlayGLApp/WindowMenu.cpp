// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2023 Darby Johnston
// All rights reserved.

#include <tlPlayGLApp/WindowMenu.h>

#include <tlPlayGLApp/App.h>
#include <tlPlayGLApp/MainWindow.h>

namespace tl
{
    namespace play_gl
    {
        struct WindowMenu::Private
        {
            std::map<std::string, std::shared_ptr<ui::Action> > actions;
            std::shared_ptr<Menu> resizeMenu;

            std::shared_ptr<observer::ValueObserver<bool> > fullScreenObserver;
            std::shared_ptr<observer::ValueObserver<bool> > floatOnTopObserver;
            std::shared_ptr<observer::ValueObserver<WindowOptions> > optionsObserver;
        };

        void WindowMenu::_init(
            const std::map<std::string, std::shared_ptr<ui::Action> >& actions,
            const std::shared_ptr<MainWindow>& mainWindow,
            const std::shared_ptr<App>& app,
            const std::shared_ptr<system::Context>& context,
            const std::shared_ptr<IWidget>& parent)
        {
            Menu::_init(context, parent);
            TLRENDER_P();

            p.actions = actions;

            p.resizeMenu = addSubMenu("Resize");
            auto appWeak = std::weak_ptr<App>(app);
            auto action = std::make_shared<ui::Action>(
                "1280x720",
                [this, appWeak]
                {
                    if (auto app = appWeak.lock())
                    {
                        app->setWindowSize(math::Size2i(1280, 720));
                    }
                });
            p.resizeMenu->addItem(action);
            action = std::make_shared<ui::Action>(
                "1920x1080",
                [this, appWeak]
                {
                    if (auto app = appWeak.lock())
                    {
                        app->setWindowSize(math::Size2i(1920, 1080));
                    }
                });
            p.resizeMenu->addItem(action);

            addDivider();
            addItem(p.actions["FullScreen"]);
            addItem(p.actions["FloatOnTop"]);
            addDivider();
            addItem(p.actions["Secondary"]);
            setItemEnabled(p.actions["Secondary"], false);
            addItem(p.actions["SecondaryFloatOnTop"]);
            setItemEnabled(p.actions["SecondaryFloatOnTop"], false);
            addDivider();
            addItem(p.actions["FileToolBar"]);
            addItem(p.actions["CompareToolBar"]);
            addItem(p.actions["WindowToolBar"]);
            addItem(p.actions["ViewToolBar"]);
            addItem(p.actions["ToolsToolBar"]);
            addItem(p.actions["Timeline"]);
            addItem(p.actions["BottomToolBar"]);
            addItem(p.actions["StatusToolBar"]);

            p.fullScreenObserver = observer::ValueObserver<bool>::create(
                app->observeFullScreen(),
                [this](bool value)
                {
                    setItemChecked(_p->actions["FullScreen"], value);
                });

            p.floatOnTopObserver = observer::ValueObserver<bool>::create(
                app->observeFloatOnTop(),
                [this](bool value)
                {
                    setItemChecked(_p->actions["FloatOnTop"], value);
                });

            p.optionsObserver = observer::ValueObserver<WindowOptions>::create(
                mainWindow->observeWindowOptions(),
                [this](const WindowOptions& value)
                {
                    setItemChecked(_p->actions["FileToolBar"], value.fileToolBar);
                    setItemChecked(_p->actions["CompareToolBar"], value.compareToolBar);
                    setItemChecked(_p->actions["WindowToolBar"], value.windowToolBar);
                    setItemChecked(_p->actions["ViewToolBar"], value.viewToolBar);
                    setItemChecked(_p->actions["ToolsToolBar"], value.toolsToolBar);
                    setItemChecked(_p->actions["Timeline"], value.timeline);
                    setItemChecked(_p->actions["BottomToolBar"], value.bottomToolBar);
                    setItemChecked(_p->actions["StatusToolBar"], value.statusToolBar);
                });
        }

        WindowMenu::WindowMenu() :
            _p(new Private)
        {}

        WindowMenu::~WindowMenu()
        {}

        std::shared_ptr<WindowMenu> WindowMenu::create(
            const std::map<std::string, std::shared_ptr<ui::Action> >& actions,
            const std::shared_ptr<MainWindow>& mainWindow,
            const std::shared_ptr<App>& app,
            const std::shared_ptr<system::Context>& context,
            const std::shared_ptr<IWidget>& parent)
        {
            auto out = std::shared_ptr<WindowMenu>(new WindowMenu);
            out->_init(actions, mainWindow, app, context, parent);
            return out;
        }

        void WindowMenu::close()
        {
            Menu::close();
            TLRENDER_P();
            p.resizeMenu->close();
        }
    }
}
