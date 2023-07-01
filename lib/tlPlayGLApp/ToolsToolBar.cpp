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
            const std::shared_ptr<system::Context>& context)
        {
            IWidget::_init("tl::examples::play_gl::ToolsToolBar", context);
            TLRENDER_P();

            p.layout = ui::HorizontalLayout::create(context, shared_from_this());
            p.layout->setSpacingRole(ui::SizeRole::None);

            p.app = app;

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
            const std::shared_ptr<system::Context>& context)
        {
            auto out = std::shared_ptr<ToolsToolBar>(new ToolsToolBar);
            out->_init(app, context);
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
