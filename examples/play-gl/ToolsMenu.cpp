// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2023 Darby Johnston
// All rights reserved.

#include "ToolsMenu.h"

#include "App.h"

namespace tl
{
    namespace examples
    {
        namespace play_gl
        {
            struct ToolsMenu::Private
            {
            };

            void ToolsMenu::_init(
                const std::shared_ptr<App>& app,
                const std::shared_ptr<system::Context>& context)
            {
                Menu::_init(context);
                TLRENDER_P();

                auto item = std::make_shared<ui::MenuItem>(
                    "Files",
                    "Files",
                    ui::Key::F1,
                    0,
                    [this](bool value)
                    {
                        close();
                    });
                addItem(item);
                setItemEnabled(item, false);

                item = std::make_shared<ui::MenuItem>(
                    "Compare",
                    "Compare",
                    ui::Key::F2,
                    0,
                    [this](bool value)
                    {
                        close();
                    });
                addItem(item);
                setItemEnabled(item, false);

                item = std::make_shared<ui::MenuItem>(
                    "Color",
                    "Color",
                    ui::Key::F3,
                    0,
                    [this](bool value)
                    {
                        close();
                    });
                addItem(item);
                setItemEnabled(item, false);

                item = std::make_shared<ui::MenuItem>(
                    "Information",
                    "Info",
                    ui::Key::F4,
                    0,
                    [this](bool value)
                    {
                        close();
                    });
                addItem(item);
                setItemEnabled(item, false);

                item = std::make_shared<ui::MenuItem>(
                    "Audio",
                    "Audio",
                    ui::Key::F5,
                    0,
                    [this](bool value)
                    {
                        close();
                    });
                addItem(item);
                setItemEnabled(item, false);

                item = std::make_shared<ui::MenuItem>(
                    "Devices",
                    "Devices",
                    ui::Key::F6,
                    0,
                    [this](bool value)
                    {
                        close();
                    });
                addItem(item);
                setItemEnabled(item, false);

                item = std::make_shared<ui::MenuItem>(
                    "Settings",
                    "Settings",
                    ui::Key::F9,
                    0,
                    [this](bool value)
                    {
                        close();
                    });
                addItem(item);
                setItemEnabled(item, false);

                item = std::make_shared<ui::MenuItem>(
                    "Messages",
                    "Messages",
                    ui::Key::F10,
                    0,
                    [this](bool value)
                    {
                        close();
                    });
                addItem(item);
                setItemEnabled(item, false);

                item = std::make_shared<ui::MenuItem>(
                    "System Log",
                    "System Log",
                    ui::Key::F11,
                    0,
                    [this](bool value)
                    {
                        close();
                    });
                addItem(item);
                setItemEnabled(item, false);
            }

            ToolsMenu::ToolsMenu() :
                _p(new Private)
            {}

            ToolsMenu::~ToolsMenu()
            {}

            std::shared_ptr<ToolsMenu> ToolsMenu::create(
                const std::shared_ptr<App>& app,
                const std::shared_ptr<system::Context>& context)
            {
                auto out = std::shared_ptr<ToolsMenu>(new ToolsMenu);
                out->_init(app, context);
                return out;
            }
        }
    }
}
