// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2023 Darby Johnston
// All rights reserved.

#include <tlPlayGLApp/WindowMenu.h>

#include <tlPlayGLApp/App.h>

namespace tl
{
    namespace play_gl
    {
        struct WindowMenu::Private
        {
            std::shared_ptr<Menu> resizeMenu;
            std::shared_ptr<ui::MenuItem> fullScreenItem;

            std::shared_ptr<observer::ValueObserver<bool> > fullScreenObserver;
        };

        void WindowMenu::_init(
            const std::shared_ptr<App>& app,
            const std::shared_ptr<system::Context>& context)
        {
            Menu::_init(context);
            TLRENDER_P();

            p.resizeMenu = addSubMenu("Resize");

            auto appWeak = std::weak_ptr<App>(app);
            auto item = std::make_shared<ui::MenuItem>(
                "1280x720",
                [this, appWeak]
                {
                    close();
                    if (auto app = appWeak.lock())
                    {
                        app->setWindowSize(imaging::Size(1280, 720));
                    }
                });
            p.resizeMenu->addItem(item);

            item = std::make_shared<ui::MenuItem>(
                "1920x1080",
                [this, appWeak]
                {
                    close();
                    if (auto app = appWeak.lock())
                    {
                        app->setWindowSize(imaging::Size(1920, 1080));
                    }
                });
            p.resizeMenu->addItem(item);

            addDivider();

            p.fullScreenItem = std::make_shared<ui::MenuItem>(
                "Full Screen",
                "WindowFullScreen",
                ui::Key::U,
                0,
                [this, appWeak](bool value)
                {
                    close();
                    if (auto app = appWeak.lock())
                    {
                        app->setFullScreen(value);
                    }
                });
            addItem(p.fullScreenItem);

            item = std::make_shared<ui::MenuItem>(
                "Float On Top",
                [this](bool value)
                {
                    close();
                });
            addItem(item);
            setItemEnabled(item, false);

            addDivider();

            item = std::make_shared<ui::MenuItem>(
                "Secondary",
                "WindowSecondary",
                [this](bool value)
                {
                    close();
                });
            addItem(item);
            setItemEnabled(item, false);

            item = std::make_shared<ui::MenuItem>(
                "Secondary Float On Top",
                [this](bool value)
                {
                    close();
                });
            addItem(item);
            setItemEnabled(item, false);

            addDivider();

            item = std::make_shared<ui::MenuItem>(
                "File Tool Bar",
                [this](bool value)
                {
                    close();
                });
            addItem(item);
            setItemEnabled(item, false);

            item = std::make_shared<ui::MenuItem>(
                "Compare Tool Bar",
                [this](bool value)
                {
                    close();
                });
            addItem(item);
            setItemEnabled(item, false);

            item = std::make_shared<ui::MenuItem>(
                "Window Tool Bar",
                [this](bool value)
                {
                    close();
                });
            addItem(item);
            setItemEnabled(item, false);

            item = std::make_shared<ui::MenuItem>(
                "View Tool Bar",
                [this](bool value)
                {
                    close();
                });
            addItem(item);
            setItemEnabled(item, false);

            item = std::make_shared<ui::MenuItem>(
                "Timeline",
                [this](bool value)
                {
                    close();
                });
            addItem(item);
            setItemEnabled(item, false);

            item = std::make_shared<ui::MenuItem>(
                "Bottom Tool Bar",
                [this](bool value)
                {
                    close();
                });
            addItem(item);
            setItemEnabled(item, false);

            p.fullScreenObserver = observer::ValueObserver<bool>::create(
                app->observeFullScreen(),
                [this](bool value)
                {
                    setItemChecked(_p->fullScreenItem, value);
                });
        }

        WindowMenu::WindowMenu() :
            _p(new Private)
        {}

        WindowMenu::~WindowMenu()
        {}

        std::shared_ptr<WindowMenu> WindowMenu::create(
            const std::shared_ptr<App>& app,
            const std::shared_ptr<system::Context>& context)
        {
            auto out = std::shared_ptr<WindowMenu>(new WindowMenu);
            out->_init(app, context);
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
