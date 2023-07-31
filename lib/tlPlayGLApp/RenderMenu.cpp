// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2023 Darby Johnston
// All rights reserved.

#include <tlPlayGLApp/RenderMenu.h>

#include <tlPlayGLApp/App.h>

namespace tl
{
    namespace play_gl
    {
        struct RenderMenu::Private
        {
            std::shared_ptr<Menu> videoLevelsMenu;
            std::shared_ptr<Menu> alphaBlendMenu;
            std::shared_ptr<Menu> minifyFilterMenu;
            std::shared_ptr<Menu> magnifyFilterMenu;
        };

        void RenderMenu::_init(
            const std::shared_ptr<App>& app,
            const std::shared_ptr<system::Context>& context,
            const std::shared_ptr<IWidget>& parent)
        {
            Menu::_init(context, parent);
            TLRENDER_P();

            auto item = std::make_shared<ui::MenuItem>(
                "Red Channel",
                ui::Key::R,
                0,
                [this]
                {
                    close();
                });
            addItem(item);
            setItemEnabled(item, false);

            item = std::make_shared<ui::MenuItem>(
                "Green Channel",
                ui::Key::G,
                0,
                [this]
                {
                    close();
                });
            addItem(item);
            setItemEnabled(item, false);

            item = std::make_shared<ui::MenuItem>(
                "Blue Channel",
                ui::Key::B,
                0,
                [this]
                {
                    close();
                });
            addItem(item);
            setItemEnabled(item, false);

            item = std::make_shared<ui::MenuItem>(
                "Alpha Channel",
                ui::Key::A,
                0,
                [this]
                {
                    close();
                });
            addItem(item);
            setItemEnabled(item, false);

            addDivider();

            item = std::make_shared<ui::MenuItem>(
                "Mirror Horizontal",
                ui::Key::H,
                0,
                [this]
                {
                    close();
                });
            addItem(item);
            setItemEnabled(item, false);

            item = std::make_shared<ui::MenuItem>(
                "Mirror Vertical",
                ui::Key::V,
                0,
                [this]
                {
                    close();
                });
            addItem(item);
            setItemEnabled(item, false);

            addDivider();

            p.videoLevelsMenu = addSubMenu("Video Levels");

            item = std::make_shared<ui::MenuItem>(
                "Full Range",
                [this](bool value)
                {
                    close();
                });
            p.videoLevelsMenu->addItem(item);
            p.videoLevelsMenu->setItemEnabled(item, false);

            item = std::make_shared<ui::MenuItem>(
                "Legal Range",
                [this](bool value)
                {
                    close();
                });
            p.videoLevelsMenu->addItem(item);
            p.videoLevelsMenu->setItemEnabled(item, false);

            p.alphaBlendMenu = addSubMenu("Alpha Blend");

            item = std::make_shared<ui::MenuItem>(
                "None",
                [this](bool value)
                {
                    close();
                });
            p.alphaBlendMenu->addItem(item);
            p.alphaBlendMenu->setItemEnabled(item, false);

            item = std::make_shared<ui::MenuItem>(
                "Straight",
                [this](bool value)
                {
                    close();
                });
            p.alphaBlendMenu->addItem(item);
            p.alphaBlendMenu->setItemEnabled(item, false);

            item = std::make_shared<ui::MenuItem>(
                "Premultiplied",
                [this](bool value)
                {
                    close();
                });
            p.alphaBlendMenu->addItem(item);
            p.alphaBlendMenu->setItemEnabled(item, false);

            p.minifyFilterMenu = addSubMenu("Minify Filter");

            item = std::make_shared<ui::MenuItem>(
                "Nearest",
                [this](bool value)
                {
                    close();
                });
            p.minifyFilterMenu->addItem(item);
            p.minifyFilterMenu->setItemEnabled(item, false);

            item = std::make_shared<ui::MenuItem>(
                "Linear",
                [this](bool value)
                {
                    close();
                });
            p.minifyFilterMenu->addItem(item);
            p.minifyFilterMenu->setItemEnabled(item, false);

            p.magnifyFilterMenu = addSubMenu("Magnify Filter");

            item = std::make_shared<ui::MenuItem>(
                "Nearest",
                [this](bool value)
                {
                    close();
                });
            p.magnifyFilterMenu->addItem(item);
            p.magnifyFilterMenu->setItemEnabled(item, false);

            item = std::make_shared<ui::MenuItem>(
                "Linear",
                [this](bool value)
                {
                    close();
                });
            p.magnifyFilterMenu->addItem(item);
            p.magnifyFilterMenu->setItemEnabled(item, false);
        }

        RenderMenu::RenderMenu() :
            _p(new Private)
        {}

        RenderMenu::~RenderMenu()
        {}

        std::shared_ptr<RenderMenu> RenderMenu::create(
            const std::shared_ptr<App>& app,
            const std::shared_ptr<system::Context>& context,
            const std::shared_ptr<IWidget>& parent)
        {
            auto out = std::shared_ptr<RenderMenu>(new RenderMenu);
            out->_init(app, context, parent);
            return out;
        }

        void RenderMenu::close()
        {
            Menu::close();
            TLRENDER_P();
            p.videoLevelsMenu->close();
            p.alphaBlendMenu->close();
            p.minifyFilterMenu->close();
            p.magnifyFilterMenu->close();
        }
    }
}
