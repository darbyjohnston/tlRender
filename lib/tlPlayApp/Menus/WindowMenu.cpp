// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2025 Darby Johnston
// All rights reserved.

#include <tlPlayApp/Menus/WindowMenu.h>

#include <tlPlayApp/App.h>
#include <tlPlayApp/MainWindow.h>

#include <dtk/core/Format.h>

namespace tl
{
    namespace play
    {
        struct WindowMenu::Private
        {
            std::map<std::string, std::shared_ptr<dtk::Action> > actions;
            std::map<std::string, std::shared_ptr<dtk::Menu> > menus;

            std::shared_ptr<dtk::ValueObserver<bool> > fullScreenObserver;
            std::shared_ptr<dtk::ValueObserver<bool> > floatOnTopObserver;
            std::shared_ptr<dtk::ValueObserver<bool> > secondaryObserver;
            std::shared_ptr<dtk::ValueObserver<WindowOptions> > optionsObserver;
        };

        void WindowMenu::_init(
            const std::shared_ptr<dtk::Context>& context,
            const std::shared_ptr<App>& app,
            const std::shared_ptr<MainWindow>& mainWindow,
            const std::map<std::string, std::shared_ptr<dtk::Action> >& actions,
            const std::shared_ptr<IWidget>& parent)
        {
            Menu::_init(context, parent);
            DTK_P();

            p.actions = actions;

            p.menus["Resize"] = addSubMenu("Resize");
            auto mainWindowWeak = std::weak_ptr<MainWindow>(mainWindow);
            const std::vector<dtk::Size2I> sizes =
            {
                dtk::Size2I(1280, 720),
                dtk::Size2I(1920, 1080),
                dtk::Size2I(3840, 2160)
            };
            for (const auto size : sizes)
            {
                auto action = std::make_shared<dtk::Action>(
                    dtk::Format("{0}x{1}").arg(size.w).arg(size.h),
                    [mainWindowWeak, size]
                    {
                        if (auto mainWindow = mainWindowWeak.lock())
                        {
                            mainWindow->setSize(size);
                        }
                    });
                p.menus["Resize"]->addItem(action);
            }

            addDivider();
            addItem(p.actions["FullScreen"]);
            addItem(p.actions["FloatOnTop"]);
            addDivider();
            addItem(p.actions["Secondary"]);
            addDivider();
            addItem(p.actions["FileToolBar"]);
            addItem(p.actions["CompareToolBar"]);
            addItem(p.actions["WindowToolBar"]);
            addItem(p.actions["ViewToolBar"]);
            addItem(p.actions["ToolsToolBar"]);
            addItem(p.actions["Timeline"]);
            addItem(p.actions["BottomToolBar"]);
            addItem(p.actions["StatusToolBar"]);

            p.fullScreenObserver = dtk::ValueObserver<bool>::create(
                mainWindow->observeFullScreen(),
                [this](bool value)
                {
                    setItemChecked(_p->actions["FullScreen"], value);
                });

            p.floatOnTopObserver = dtk::ValueObserver<bool>::create(
                mainWindow->observeFloatOnTop(),
                [this](bool value)
                {
                    setItemChecked(_p->actions["FloatOnTop"], value);
                });

            p.secondaryObserver = dtk::ValueObserver<bool>::create(
                app->observeSecondaryWindow(),
                [this](bool value)
                {
                    setItemChecked(_p->actions["Secondary"], value);
                });

            p.optionsObserver = dtk::ValueObserver<WindowOptions>::create(
                app->getSettingsModel()->observeWindow(),
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
            const std::shared_ptr<dtk::Context>& context,
            const std::shared_ptr<App>& app,
            const std::shared_ptr<MainWindow>& mainWindow,
            const std::map<std::string, std::shared_ptr<dtk::Action> >& actions,
            const std::shared_ptr<IWidget>& parent)
        {
            auto out = std::shared_ptr<WindowMenu>(new WindowMenu);
            out->_init(context, app, mainWindow, actions, parent);
            return out;
        }

        void WindowMenu::close()
        {
            Menu::close();
            DTK_P();
            for (const auto menu : p.menus)
            {
                menu.second->close();
            }
        }
    }
}
