// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2025 Darby Johnston
// All rights reserved.

#include <tlPlayApp/WindowToolBar.h>

#include <tlPlayApp/App.h>
#include <tlPlayApp/MainWindow.h>

#include <dtk/ui/RowLayout.h>
#include <dtk/ui/ToolButton.h>

namespace tl
{
    namespace play_app
    {
        struct WindowToolBar::Private
        {
            std::weak_ptr<App> app;

            std::map<std::string, std::shared_ptr<dtk::Action> > actions;
            std::map<std::string, std::shared_ptr<dtk::ToolButton> > buttons;
            std::shared_ptr<dtk::HorizontalLayout> layout;

            std::shared_ptr<dtk::ValueObserver<bool> > fullScreenObserver;
            std::shared_ptr<dtk::ValueObserver<bool> > secondaryObserver;
        };

        void WindowToolBar::_init(
            const std::shared_ptr<dtk::Context>& context,
            const std::shared_ptr<App>& app,
            const std::shared_ptr<MainWindow>& mainWindow,
            const std::map<std::string, std::shared_ptr<dtk::Action> >& actions,
            const std::shared_ptr<IWidget>& parent)
        {
            IWidget::_init(
                context,
                "tl::play_app::WindowToolBar",
                parent);
            DTK_P();

            p.app = app;
            p.actions = actions;

            p.buttons["FullScreen"] = dtk::ToolButton::create(context);
            p.buttons["FullScreen"]->setIcon(p.actions["FullScreen"]->icon);
            p.buttons["FullScreen"]->setCheckable(p.actions["FullScreen"]->checkable);
            p.buttons["FullScreen"]->setTooltip(p.actions["FullScreen"]->toolTip);

            p.buttons["Secondary"] = dtk::ToolButton::create(context);
            p.buttons["Secondary"]->setIcon(p.actions["Secondary"]->icon);
            p.buttons["Secondary"]->setCheckable(p.actions["Secondary"]->checkable);
            p.buttons["Secondary"]->setTooltip(p.actions["Secondary"]->toolTip);

            p.layout = dtk::HorizontalLayout::create(context, shared_from_this());
            p.layout->setSpacingRole(dtk::SizeRole::None);
            p.buttons["FullScreen"]->setParent(p.layout);
            p.buttons["Secondary"]->setParent(p.layout);

            auto mainWindowWeak = std::weak_ptr<MainWindow>(mainWindow);
            p.buttons["FullScreen"]->setCheckedCallback(
                [mainWindowWeak](bool value)
                {
                    if (auto mainWindow = mainWindowWeak.lock())
                    {
                        mainWindow->setFullScreen(value);
                    }
                });
            auto appWeak = std::weak_ptr<App>(app);
            p.buttons["Secondary"]->setCheckedCallback(
                [appWeak](bool value)
                {
                    if (auto app = appWeak.lock())
                    {
                        app->setSecondaryWindow(value);
                    }
                });

            p.fullScreenObserver = dtk::ValueObserver<bool>::create(
                mainWindow->observeFullScreen(),
                [this](bool value)
                {
                    _p->buttons["FullScreen"]->setChecked(value);
                });

            p.secondaryObserver = dtk::ValueObserver<bool>::create(
                app->observeSecondaryWindow(),
                [this](bool value)
                {
                    _p->buttons["Secondary"]->setChecked(value);
                });
        }

        WindowToolBar::WindowToolBar() :
            _p(new Private)
        {}

        WindowToolBar::~WindowToolBar()
        {}

        std::shared_ptr<WindowToolBar> WindowToolBar::create(
            const std::shared_ptr<dtk::Context>& context,
            const std::shared_ptr<App>& app,
            const std::shared_ptr<MainWindow>& mainWindow,
            const std::map<std::string, std::shared_ptr<dtk::Action> >& actions,
            const std::shared_ptr<IWidget>& parent)
        {
            auto out = std::shared_ptr<WindowToolBar>(new WindowToolBar);
            out->_init(context, app, mainWindow, actions, parent);
            return out;
        }

        void WindowToolBar::setGeometry(const dtk::Box2I& value)
        {
            IWidget::setGeometry(value);
            _p->layout->setGeometry(value);
        }

        void WindowToolBar::sizeHintEvent(const dtk::SizeHintEvent& event)
        {
            IWidget::sizeHintEvent(event);
            _setSizeHint(_p->layout->getSizeHint());
        }
    }
}
