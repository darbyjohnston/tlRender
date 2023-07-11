// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2023 Darby Johnston
// All rights reserved.

#include <tlPlayGLApp/ToolsToolBar.h>

#include <tlPlayGLApp/App.h>

#include <tlUI/RowLayout.h>
#include <tlUI/ToolButton.h>

namespace tl
{
    namespace play_gl
    {
        struct ToolsToolBar::Private
        {
            std::weak_ptr<App> app;
            std::shared_ptr<timeline::Player> player;

            std::map<std::string, std::shared_ptr<ui::ToolButton> > buttons;
            std::shared_ptr<ui::HorizontalLayout> layout;

            std::shared_ptr<observer::ListObserver<std::shared_ptr<timeline::Player> > > playerObserver;
        };

        void ToolsToolBar::_init(
            const std::shared_ptr<App>& app,
            const std::shared_ptr<system::Context>& context,
            const std::shared_ptr<IWidget>& parent)
        {
            IWidget::_init(
                "tl::play_gl::ToolsToolBar",
                context,
                parent);
            TLRENDER_P();

            p.app = app;

            p.buttons["Files"] = ui::ToolButton::create(context);
            p.buttons["Files"]->setIcon("Files");
            p.buttons["Files"]->setCheckable(true);
            p.buttons["Files"]->setEnabled(false);

            p.buttons["Compare"] = ui::ToolButton::create(context);
            p.buttons["Compare"]->setIcon("Compare");
            p.buttons["Compare"]->setCheckable(true);
            p.buttons["Compare"]->setEnabled(false);

            p.buttons["Color"] = ui::ToolButton::create(context);
            p.buttons["Color"]->setIcon("Color");
            p.buttons["Color"]->setCheckable(true);
            p.buttons["Color"]->setEnabled(false);

            p.buttons["Info"] = ui::ToolButton::create(context);
            p.buttons["Info"]->setIcon("Info");
            p.buttons["Info"]->setCheckable(true);
            p.buttons["Info"]->setEnabled(false);

            p.buttons["Audio"] = ui::ToolButton::create(context);
            p.buttons["Audio"]->setIcon("Audio");
            p.buttons["Audio"]->setCheckable(true);
            p.buttons["Audio"]->setEnabled(false);

            p.buttons["Devices"] = ui::ToolButton::create(context);
            p.buttons["Devices"]->setIcon("Devices");
            p.buttons["Devices"]->setCheckable(true);
            p.buttons["Devices"]->setEnabled(false);

            p.buttons["Settings"] = ui::ToolButton::create(context);
            p.buttons["Settings"]->setIcon("Settings");
            p.buttons["Settings"]->setCheckable(true);
            p.buttons["Settings"]->setEnabled(false);

            p.buttons["Messages"] = ui::ToolButton::create(context);
            p.buttons["Messages"]->setIcon("Messages");
            p.buttons["Messages"]->setCheckable(true);
            p.buttons["Messages"]->setEnabled(false);

            p.layout = ui::HorizontalLayout::create(context, shared_from_this());
            p.layout->setSpacingRole(ui::SizeRole::None);
            p.buttons["Files"]->setParent(p.layout);
            p.buttons["Compare"]->setParent(p.layout);
            p.buttons["Color"]->setParent(p.layout);
            p.buttons["Info"]->setParent(p.layout);
            p.buttons["Audio"]->setParent(p.layout);
            p.buttons["Devices"]->setParent(p.layout);
            p.buttons["Settings"]->setParent(p.layout);
            p.buttons["Messages"]->setParent(p.layout);

            auto appWeak = std::weak_ptr<App>(app);
            p.playerObserver = observer::ListObserver<std::shared_ptr<timeline::Player> >::create(
                app->observeActivePlayers(),
                [this](const std::vector<std::shared_ptr<timeline::Player> >& value)
                {
                    _p->player = !value.empty() ? value[0] : nullptr;
                });
        }

        ToolsToolBar::ToolsToolBar() :
            _p(new Private)
        {}

        ToolsToolBar::~ToolsToolBar()
        {}

        std::shared_ptr<ToolsToolBar> ToolsToolBar::create(
            const std::shared_ptr<App>& app,
            const std::shared_ptr<system::Context>& context,
            const std::shared_ptr<IWidget>& parent)
        {
            auto out = std::shared_ptr<ToolsToolBar>(new ToolsToolBar);
            out->_init(app, context, parent);
            return out;
        }

        void ToolsToolBar::setGeometry(const math::BBox2i& value)
        {
            IWidget::setGeometry(value);
            _p->layout->setGeometry(value);
        }

        void ToolsToolBar::sizeHintEvent(const ui::SizeHintEvent& event)
        {
            IWidget::sizeHintEvent(event);
            _sizeHint = _p->layout->getSizeHint();
        }
    }
}
