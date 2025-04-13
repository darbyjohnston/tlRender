// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2024 Darby Johnston
// All rights reserved.

#include "MenuBar.h"

#include "App.h"

namespace tl
{
    namespace examples
    {
        namespace player
        {
            void MenuBar::_init(
                const std::shared_ptr<dtk::Context>& context,
                const std::shared_ptr<App>& app)
            {
                dtk::MenuBar::_init(context, nullptr);

                auto appWeak = std::weak_ptr<App>(app);
                _actions["File/Open"] = dtk::Action::create(
                    "Open",
                    "FileOpen",
                    dtk::Key::O,
                    static_cast<int>(dtk::commandKeyModifier),
                    [appWeak]
                    {
                        if (auto app = appWeak.lock())
                        {
                            app->open();
                        }
                    });
                _actions["File/Close"] = dtk::Action::create(
                    "Close",
                    "FileClose",
                    dtk::Key::E,
                    static_cast<int>(dtk::commandKeyModifier),
                    [appWeak]
                    {
                        if (auto app = appWeak.lock())
                        {
                            app->close();
                        }
                    });
                _actions["File/Reload"] = dtk::Action::create(
                    "Reload",
                    "FileReload",
                    dtk::Key::R,
                    static_cast<int>(dtk::commandKeyModifier),
                    [appWeak]
                    {
                        if (auto app = appWeak.lock())
                        {
                            app->reload();
                        }
                    });
                _actions["File/Exit"] = dtk::Action::create(
                    "Exit",
                    dtk::Key::Q,
                    static_cast<int>(dtk::commandKeyModifier),
                    [appWeak]
                    {
                        if (auto app = appWeak.lock())
                        {
                            app->exit();
                        }
                    });

                auto fileMenu = dtk::Menu::create(context);
                fileMenu->addItem(_actions["File/Open"]);
                fileMenu->addItem(_actions["File/Close"]);
                fileMenu->addItem(_actions["File/Reload"]);
                fileMenu->addDivider();
                fileMenu->addItem(_actions["File/Exit"]);
                addMenu("File", fileMenu);
            }

            MenuBar::~MenuBar()
            {}

            std::shared_ptr<MenuBar> MenuBar::create(
                const std::shared_ptr<dtk::Context>& context,
                const std::shared_ptr<App>& app)
            {
                auto out = std::shared_ptr<MenuBar>(new MenuBar);
                out->_init(context, app);
                return out;
            }

            const std::map<std::string, std::shared_ptr<dtk::Action> >& MenuBar::getActions() const
            {
                return _actions;
            }
        }
    }
}
