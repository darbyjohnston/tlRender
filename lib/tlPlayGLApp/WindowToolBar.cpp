// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2023 Darby Johnston
// All rights reserved.

#include <tlPlayGLApp/WindowToolBar.h>

#include <tlPlayGLApp/App.h>
#include <tlPlayGLApp/MainWindow.h>

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

            std::map<std::string, std::shared_ptr<ui::Action> > actions;
            std::map<std::string, std::shared_ptr<ui::ToolButton> > buttons;
            std::shared_ptr<ui::HorizontalLayout> layout;

            std::shared_ptr<observer::ListObserver<std::shared_ptr<timeline::Player> > > playerObserver;
            std::shared_ptr<observer::ValueObserver<bool> > fullScreenObserver;
        };

        void WindowToolBar::_init(
            const std::map<std::string, std::shared_ptr<ui::Action> >& actions,
            const std::shared_ptr<App>& app,
            const std::shared_ptr<system::Context>& context,
            const std::shared_ptr<IWidget>& parent)
        {
            IWidget::_init(
                "tl::play_gl::WindowToolBar",
                context,
                parent);
            TLRENDER_P();

            p.app = app;
            p.actions = actions;

            p.buttons["FullScreen"] = ui::ToolButton::create(context);
            p.buttons["FullScreen"]->setIcon(p.actions["FullScreen"]->icon);
            p.buttons["FullScreen"]->setCheckable(p.actions["FullScreen"]->checkable);
            p.buttons["FullScreen"]->setToolTip(p.actions["FullScreen"]->toolTip);

            p.buttons["Secondary"] = ui::ToolButton::create(context);
            p.buttons["Secondary"]->setIcon(p.actions["Secondary"]->icon);
            p.buttons["Secondary"]->setCheckable(p.actions["Secondary"]->checkable);
            p.buttons["Secondary"]->setToolTip(p.actions["Secondary"]->toolTip);

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
                        app->getMainWindow()->setFullScreen(value);
                    }
                });
            p.buttons["Secondary"]->setCheckedCallback(
                [appWeak](bool value)
                {
                    if (auto app = appWeak.lock())
                    {
                        app->getMainWindow()->setSecondaryWindow(value);
                    }
                });

            p.playerObserver = observer::ListObserver<std::shared_ptr<timeline::Player> >::create(
                app->observeActivePlayers(),
                [this](const std::vector<std::shared_ptr<timeline::Player> >& value)
                {
                    _p->player = !value.empty() ? value[0] : nullptr;
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
            const std::map<std::string, std::shared_ptr<ui::Action> >& actions,
            const std::shared_ptr<App>& app,
            const std::shared_ptr<system::Context>& context,
            const std::shared_ptr<IWidget>& parent)
        {
            auto out = std::shared_ptr<WindowToolBar>(new WindowToolBar);
            out->_init(actions, app, context, parent);
            return out;
        }

        void WindowToolBar::setGeometry(const math::Box2i& value)
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
