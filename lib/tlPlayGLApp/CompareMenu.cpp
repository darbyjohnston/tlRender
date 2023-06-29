// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2023 Darby Johnston
// All rights reserved.

#include <tlPlayGLApp/CompareMenu.h>

#include <tlPlayGLApp/App.h>

namespace tl
{
    namespace play_gl
    {
        struct CompareMenu::Private
        {
        };

        void CompareMenu::_init(
            const std::shared_ptr<App>& app,
            const std::shared_ptr<system::Context>& context)
        {
            Menu::_init(context);
            TLRENDER_P();

            auto item = std::make_shared<ui::MenuItem>(
                "A",
                "CompareA",
                ui::Key::A,
                static_cast<int>(ui::KeyModifier::Control),
                [this]
                {
                    close();
                });
            addItem(item);
            setItemEnabled(item, false);

            item = std::make_shared<ui::MenuItem>(
                "B",
                "CompareB",
                ui::Key::B,
                static_cast<int>(ui::KeyModifier::Control),
                [this]
                {
                    close();
                });
            addItem(item);
            setItemEnabled(item, false);

            item = std::make_shared<ui::MenuItem>(
                "Wipe",
                "CompareWipe",
                ui::Key::W,
                static_cast<int>(ui::KeyModifier::Control),
                [this]
                {
                    close();
                });
            addItem(item);
            setItemEnabled(item, false);

            item = std::make_shared<ui::MenuItem>(
                "Overlay",
                "CompareOverlay",
                [this]
                {
                    close();
                });
            addItem(item);
            setItemEnabled(item, false);

            item = std::make_shared<ui::MenuItem>(
                "Difference",
                "CompareDifference",
                [this]
                {
                    close();
                });
            addItem(item);
            setItemEnabled(item, false);

            item = std::make_shared<ui::MenuItem>(
                "Horizontal",
                "CompareHorizontal",
                [this]
                {
                    close();
                });
            addItem(item);
            setItemEnabled(item, false);

            item = std::make_shared<ui::MenuItem>(
                "Vertical",
                "CompareVertical",
                [this]
                {
                    close();
                });
            addItem(item);
            setItemEnabled(item, false);

            item = std::make_shared<ui::MenuItem>(
                "Tile",
                "CompareTile",
                ui::Key::T,
                static_cast<int>(ui::KeyModifier::Control),
                [this]
                {
                    close();
                });
            addItem(item);
            setItemEnabled(item, false);

            addDivider();

            item = std::make_shared<ui::MenuItem>(
                "Next",
                "Next",
                ui::Key::PageDown,
                static_cast<int>(ui::KeyModifier::Shift),
                [this]
                {
                    close();
                });
            addItem(item);
            setItemEnabled(item, false);

            item = std::make_shared<ui::MenuItem>(
                "Previous",
                "Prev",
                ui::Key::PageUp,
                static_cast<int>(ui::KeyModifier::Shift),
                [this]
                {
                    close();
                });
            addItem(item);
            setItemEnabled(item, false);
        }

        CompareMenu::CompareMenu() :
            _p(new Private)
        {}

        CompareMenu::~CompareMenu()
        {}

        std::shared_ptr<CompareMenu> CompareMenu::create(
            const std::shared_ptr<App>& app,
            const std::shared_ptr<system::Context>& context)
        {
            auto out = std::shared_ptr<CompareMenu>(new CompareMenu);
            out->_init(app, context);
            return out;
        }
    }
}
