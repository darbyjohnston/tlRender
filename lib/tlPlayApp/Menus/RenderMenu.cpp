// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2025 Darby Johnston
// All rights reserved.

#include <tlPlayApp/Menus/RenderMenu.h>

#include <tlPlayApp/Actions/RenderActions.h>
#include <tlPlayApp/Models/RenderModel.h>
#include <tlPlayApp/App.h>

#include <sstream>

namespace tl
{
    namespace play
    {
        struct RenderMenu::Private
        {
            std::map<std::string, std::shared_ptr<dtk::Action> > actions;
            std::map<std::string, std::shared_ptr<Menu> > menus;

            std::shared_ptr<dtk::ValueObserver<dtk::ImageOptions> > imageOptionsObserver;
            std::shared_ptr<dtk::ValueObserver<dtk::ImageType> > colorBufferObserver;
        };

        void RenderMenu::_init(
            const std::shared_ptr<dtk::Context>& context,
            const std::shared_ptr<App>& app,
            const std::shared_ptr<RenderActions>& actions,
            const std::shared_ptr<IWidget>& parent)
        {
            Menu::_init(context, parent);
            DTK_P();

            p.actions = actions->getActions();

            p.menus["VideoLevels"] = addSubMenu("Video Levels");
            p.menus["VideoLevels"]->addItem(p.actions["FromFile"]);
            p.menus["VideoLevels"]->addItem(p.actions["FullRange"]);
            p.menus["VideoLevels"]->addItem(p.actions["LegalRange"]);

            p.menus["AlphaBlend"] = addSubMenu("Alpha Blend");
            p.menus["AlphaBlend"]->addItem(p.actions["AlphaBlendNone"]);
            p.menus["AlphaBlend"]->addItem(p.actions["AlphaBlendStraight"]);
            p.menus["AlphaBlend"]->addItem(p.actions["AlphaBlendPremultiplied"]);

            p.menus["ColorBuffer"] = addSubMenu("Color Buffer");
            std::vector<dtk::ImageType> colorBuffers =
                actions->getColorBuffers();
            for (auto type : colorBuffers)
            {
                std::stringstream ss;
                ss << type;
                p.menus["ColorBuffer"]->addItem(p.actions[ss.str()]);
            }

            p.imageOptionsObserver = dtk::ValueObserver<dtk::ImageOptions>::create(
                app->getRenderModel()->observeImageOptions(),
                [this](const dtk::ImageOptions& value)
                {
                    _p->menus["VideoLevels"]->setItemChecked(
                        _p->actions["FromFile"],
                        dtk::InputVideoLevels::FromFile == value.videoLevels);
                    _p->menus["VideoLevels"]->setItemChecked(
                        _p->actions["FullRange"],
                        dtk::InputVideoLevels::FullRange == value.videoLevels);
                    _p->menus["VideoLevels"]->setItemChecked(
                        _p->actions["LegalRange"],
                        dtk::InputVideoLevels::LegalRange == value.videoLevels);

                    _p->menus["AlphaBlend"]->setItemChecked(
                        _p->actions["AlphaBlendNone"],
                        dtk::AlphaBlend::None == value.alphaBlend);
                    _p->menus["AlphaBlend"]->setItemChecked(
                        _p->actions["AlphaBlendStraight"],
                        dtk::AlphaBlend::Straight == value.alphaBlend);
                    _p->menus["AlphaBlend"]->setItemChecked(
                        _p->actions["AlphaBlendPremultiplied"],
                        dtk::AlphaBlend::Premultiplied == value.alphaBlend);
                });

            p.colorBufferObserver = dtk::ValueObserver<dtk::ImageType>::create(
                app->getRenderModel()->observeColorBuffer(),
                [this, colorBuffers](dtk::ImageType value)
                {
                    for (auto type : colorBuffers)
                    {
                        std::stringstream ss;
                        ss << type;
                        _p->menus["ColorBuffer"]->setItemChecked(
                            _p->actions[ss.str()],
                            type == value);
                    }
                });
        }

        RenderMenu::RenderMenu() :
            _p(new Private)
        {}

        RenderMenu::~RenderMenu()
        {}

        std::shared_ptr<RenderMenu> RenderMenu::create(
            const std::shared_ptr<dtk::Context>& context,
            const std::shared_ptr<App>& app,
            const std::shared_ptr<RenderActions>& actions,
            const std::shared_ptr<IWidget>& parent)
        {
            auto out = std::shared_ptr<RenderMenu>(new RenderMenu);
            out->_init(context, app, actions, parent);
            return out;
        }

        void RenderMenu::close()
        {
            Menu::close();
            DTK_P();
            for (const auto& menu : p.menus)
            {
                menu.second->close();
            }
        }
    }
}
