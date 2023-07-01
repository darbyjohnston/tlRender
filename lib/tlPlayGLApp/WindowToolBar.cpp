// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2023 Darby Johnston
// All rights reserved.

#include <tlPlayGLApp/WindowToolBar.h>

#include <tlPlayGLApp/App.h>

#include <tlUI/RowLayout.h>
#include <tlUI/ToolButton.h>

namespace tl
{
    namespace play_gl
    {
        struct WindowToolBar::Private
        {
            std::weak_ptr<App> app;
            std::shared_ptr<timeline::Player> player;

            std::map<std::string, std::shared_ptr<ui::ToolButton> > buttons;
            std::shared_ptr<ui::HorizontalLayout> layout;

            std::shared_ptr<observer::ValueObserver<std::shared_ptr<timeline::Player> > > playerObserver;
            std::shared_ptr<observer::ValueObserver<bool> > fullScreenObserver;
        };

        void WindowToolBar::_init(
            const std::shared_ptr<App>& app,
            const std::shared_ptr<system::Context>& context)
        {
            IWidget::_init("tl::examples::play_gl::WindowToolBar", context);
            TLRENDER_P();

            p.app = app;

            p.buttons["FullScreen"] = ui::ToolButton::create(context);
            p.buttons["FullScreen"]->setIcon("WindowFullScreen");
            p.buttons["FullScreen"]->setCheckable(true);

            p.buttons["Secondary"] = ui::ToolButton::create(context);
            p.buttons["Secondary"]->setIcon("WindowSecondary");
            p.buttons["Secondary"]->setEnabled(false);

            p.layout = ui::HorizontalLayout::create(context, shared_from_this());
            p.layout->setSpacingRole(ui::SizeRole::None);
            p.buttons["FullScreen"]->setParent(p.layout);
            p.buttons["Secondary"]->setParent(p.layout);

            auto appWeak = std::weak_ptr<App>(app);
            p.buttons["FullScreen"]->setCheckedCallback(
                [appWeak](bool value)
                {
                    if (auto app = appWeak.lock())
                    {
                        app->setFullScreen(value);
                    }
                });

            p.playerObserver = observer::ValueObserver<std::shared_ptr<timeline::Player> >::create(
                app->observePlayer(),
                [this](const std::shared_ptr<timeline::Player>& value)
                {
                    _p->player = value;
                });

            p.fullScreenObserver = observer::ValueObserver<bool>::create(
                app->observeFullScreen(),
                [this](bool value)
                {
                    _p->buttons["FullScreen"]->setChecked(value);
                });
        }

        WindowToolBar::WindowToolBar() :
            _p(new Private)
        {}

        WindowToolBar::~WindowToolBar()
        {}

        std::shared_ptr<WindowToolBar> WindowToolBar::create(
            const std::shared_ptr<App>& app,
            const std::shared_ptr<system::Context>& context)
        {
            auto out = std::shared_ptr<WindowToolBar>(new WindowToolBar);
            out->_init(app, context);
            return out;
        }

        void WindowToolBar::setGeometry(const math::BBox2i& value)
        {
            IWidget::setGeometry(value);
            _p->layout->setGeometry(value);
        }

        void WindowToolBar::sizeHintEvent(const ui::SizeHintEvent& event)
        {
            IWidget::sizeHintEvent(event);
            _sizeHint = _p->layout->getSizeHint();
        }
    }
}
