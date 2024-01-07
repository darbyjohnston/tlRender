// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2023 Darby Johnston
// All rights reserved.

#include <tlPlayApp/RenderMenu.h>

#include <tlPlayApp/App.h>

#include <tlPlay/ColorModel.h>

namespace tl
{
    namespace play_app
    {
        struct RenderMenu::Private
        {
            std::map<std::string, std::shared_ptr<ui::Action> > actions;
            std::shared_ptr<Menu> videoLevelsMenu;
            std::shared_ptr<Menu> alphaBlendMenu;
            std::shared_ptr<Menu> minifyFilterMenu;
            std::shared_ptr<Menu> magnifyFilterMenu;

            std::shared_ptr<observer::ValueObserver<timeline::ImageOptions> > imageOptionsObserver;
            std::shared_ptr<observer::ValueObserver<timeline::DisplayOptions> > displayOptionsObserver;
        };

        void RenderMenu::_init(
            const std::map<std::string, std::shared_ptr<ui::Action> >& actions,
            const std::shared_ptr<App>& app,
            const std::shared_ptr<system::Context>& context,
            const std::shared_ptr<IWidget>& parent)
        {
            Menu::_init(context, parent);
            TLRENDER_P();

            p.actions = actions;

            addItem(p.actions["Red"]);
            addItem(p.actions["Green"]);
            addItem(p.actions["Blue"]);
            addItem(p.actions["Alpha"]);
            addDivider();
            addItem(p.actions["MirrorHorizontal"]);
            addItem(p.actions["MirrorVertical"]);
            addDivider();

            p.videoLevelsMenu = addSubMenu("Video Levels");
            p.videoLevelsMenu->addItem(p.actions["FromFile"]);
            p.videoLevelsMenu->addItem(p.actions["FullRange"]);
            p.videoLevelsMenu->addItem(p.actions["LegalRange"]);

            p.alphaBlendMenu = addSubMenu("Alpha Blend");
            p.alphaBlendMenu->addItem(p.actions["AlphaBlendNone"]);
            p.alphaBlendMenu->addItem(p.actions["AlphaBlendStraight"]);
            p.alphaBlendMenu->addItem(p.actions["AlphaBlendPremultiplied"]);

            p.minifyFilterMenu = addSubMenu("Minify Filter");
            p.minifyFilterMenu->addItem(p.actions["MinifyNearest"]);
            p.minifyFilterMenu->addItem(p.actions["MinifyLinear"]);

            p.magnifyFilterMenu = addSubMenu("Magnify Filter");
            p.magnifyFilterMenu->addItem(p.actions["MagnifyNearest"]);
            p.magnifyFilterMenu->addItem(p.actions["MagnifyLinear"]);

            p.imageOptionsObserver = observer::ValueObserver<timeline::ImageOptions>::create(
                app->getColorModel()->observeImageOptions(),
                [this](const timeline::ImageOptions& value)
                {
                    _p->videoLevelsMenu->setItemChecked(
                        _p->actions["FromFile"],
                        timeline::InputVideoLevels::FromFile == value.videoLevels);
                    _p->videoLevelsMenu->setItemChecked(
                        _p->actions["FullRange"],
                        timeline::InputVideoLevels::FullRange == value.videoLevels);
                    _p->videoLevelsMenu->setItemChecked(
                        _p->actions["LegalRange"],
                        timeline::InputVideoLevels::LegalRange == value.videoLevels);

                    _p->alphaBlendMenu->setItemChecked(
                        _p->actions["AlphaBlendNone"],
                        timeline::AlphaBlend::None == value.alphaBlend);
                    _p->alphaBlendMenu->setItemChecked(
                        _p->actions["AlphaBlendStraight"],
                        timeline::AlphaBlend::Straight == value.alphaBlend);
                    _p->alphaBlendMenu->setItemChecked(
                        _p->actions["AlphaBlendPremultiplied"],
                        timeline::AlphaBlend::Premultiplied == value.alphaBlend);
                });

            p.displayOptionsObserver = observer::ValueObserver<timeline::DisplayOptions>::create(
                app->getColorModel()->observeDisplayOptions(),
                [this](const timeline::DisplayOptions& value)
                {
                    setItemChecked(
                        _p->actions["Red"],
                        timeline::Channels::Red == value.channels);
                    setItemChecked(
                        _p->actions["Green"],
                        timeline::Channels::Green == value.channels);
                    setItemChecked(
                        _p->actions["Blue"],
                        timeline::Channels::Blue == value.channels);
                    setItemChecked(
                        _p->actions["Alpha"],
                        timeline::Channels::Alpha == value.channels);

                    setItemChecked(
                        _p->actions["MirrorHorizontal"],
                        value.mirror.x);
                    setItemChecked(
                        _p->actions["MirrorVertical"],
                        value.mirror.y);

                    _p->minifyFilterMenu->setItemChecked(
                        _p->actions["MinifyNearest"],
                        timeline::ImageFilter::Nearest == value.imageFilters.minify);
                    _p->minifyFilterMenu->setItemChecked(
                        _p->actions["MinifyLinear"],
                        timeline::ImageFilter::Linear == value.imageFilters.minify);

                    _p->magnifyFilterMenu->setItemChecked(
                        _p->actions["MagnifyNearest"],
                        timeline::ImageFilter::Nearest == value.imageFilters.magnify);
                    _p->magnifyFilterMenu->setItemChecked(
                        _p->actions["MagnifyLinear"],
                        timeline::ImageFilter::Linear == value.imageFilters.magnify);
                });
        }

        RenderMenu::RenderMenu() :
            _p(new Private)
        {}

        RenderMenu::~RenderMenu()
        {}

        std::shared_ptr<RenderMenu> RenderMenu::create(
            const std::map<std::string, std::shared_ptr<ui::Action> >& actions,
            const std::shared_ptr<App>& app,
            const std::shared_ptr<system::Context>& context,
            const std::shared_ptr<IWidget>& parent)
        {
            auto out = std::shared_ptr<RenderMenu>(new RenderMenu);
            out->_init(actions, app, context, parent);
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
