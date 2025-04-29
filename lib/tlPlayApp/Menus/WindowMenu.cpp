// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2025 Darby Johnston
// All rights reserved.

#include <tlPlayApp/Menus/WindowMenu.h>

#include <tlPlayApp/Actions/WindowActions.h>
#include <tlPlayApp/MainWindow.h>

#include <dtk/core/Format.h>

namespace tl
{
    namespace play
    {
        struct WindowMenu::Private
        {
            std::map<std::string, std::shared_ptr<dtk::Menu> > menus;
        };

        void WindowMenu::_init(
            const std::shared_ptr<dtk::Context>& context,
            const std::shared_ptr<MainWindow>& mainWindow,
            const std::shared_ptr<WindowActions>& windowActions,
            const std::shared_ptr<IWidget>& parent)
        {
            Menu::_init(context, parent);
            DTK_P();

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
                auto action = dtk::Action::create(
                    dtk::Format("{0}x{1}").arg(size.w).arg(size.h),
                    [mainWindowWeak, size]
                    {
                        if (auto mainWindow = mainWindowWeak.lock())
                        {
                            mainWindow->setSize(size);
                        }
                    });
                p.menus["Resize"]->addAction(action);
            }

            addDivider();
            auto actions = windowActions->getActions();
            addAction(actions["FullScreen"]);
            addAction(actions["FloatOnTop"]);
            addDivider();
            addAction(actions["Secondary"]);
            addDivider();
            addAction(actions["FileToolBar"]);
            addAction(actions["CompareToolBar"]);
            addAction(actions["WindowToolBar"]);
            addAction(actions["ViewToolBar"]);
            addAction(actions["ToolsToolBar"]);
            addAction(actions["TabBar"]);
            addAction(actions["Timeline"]);
            addAction(actions["BottomToolBar"]);
            addAction(actions["StatusToolBar"]);
        }

        WindowMenu::WindowMenu() :
            _p(new Private)
        {}

        WindowMenu::~WindowMenu()
        {}

        std::shared_ptr<WindowMenu> WindowMenu::create(
            const std::shared_ptr<dtk::Context>& context,
            const std::shared_ptr<MainWindow>& mainWindow,
            const std::shared_ptr<WindowActions>& windowActions,
            const std::shared_ptr<IWidget>& parent)
        {
            auto out = std::shared_ptr<WindowMenu>(new WindowMenu);
            out->_init(context, mainWindow, windowActions, parent);
            return out;
        }
    }
}
