// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2025 Darby Johnston
// All rights reserved.

#include <tlPlayApp/Widgets/WindowToolBar.h>

#include <tlPlayApp/Widgets/ToolBarButton.h>
#include <tlPlayApp/App.h>
#include <tlPlayApp/MainWindow.h>

#include <dtk/ui/RowLayout.h>
#include <dtk/ui/ToolButton.h>

namespace tl
{
    namespace play
    {
        struct WindowToolBar::Private
        {
            std::map<std::string, std::shared_ptr<dtk::Action> > actions;
            std::map<std::string, std::shared_ptr<ToolBarButton> > buttons;
            std::shared_ptr<dtk::HorizontalLayout> layout;
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

            p.actions = actions;

            p.buttons["FullScreen"] = ToolBarButton::create(context, p.actions["FullScreen"]);
            p.buttons["Secondary"] = ToolBarButton::create(context, p.actions["Secondary"]);

            p.layout = dtk::HorizontalLayout::create(context, shared_from_this());
            p.layout->setSpacingRole(dtk::SizeRole::None);
            p.buttons["FullScreen"]->setParent(p.layout);
            p.buttons["Secondary"]->setParent(p.layout);
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
