// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2025 Darby Johnston
// All rights reserved.

#include <tlPlayApp/ViewToolBar.h>

#include <tlPlayApp/App.h>
#include <tlPlayApp/MainWindow.h>

#include <tlPlay/Viewport.h>

#include <dtk/ui/RowLayout.h>
#include <dtk/ui/ToolButton.h>

namespace tl
{
    namespace play_app
    {
        struct ViewToolBar::Private
        {
            std::weak_ptr<App> app;

            std::map<std::string, std::shared_ptr<dtk::Action> > actions;
            std::map<std::string, std::shared_ptr<dtk::ToolButton> > buttons;
            std::shared_ptr<dtk::HorizontalLayout> layout;

            std::shared_ptr<dtk::ValueObserver<bool> > frameViewObserver;
        };

        void ViewToolBar::_init(
            const std::shared_ptr<dtk::Context>& context,
            const std::shared_ptr<App>& app,
            const std::shared_ptr<MainWindow>& mainWindow,
            const std::map<std::string, std::shared_ptr<dtk::Action> >& actions,
            const std::shared_ptr<IWidget>& parent)
        {
            IWidget::_init(
                context,
                "tl::play_app::ViewToolBar",
                parent);
            DTK_P();

            p.app = app;
            p.actions = actions;

            p.buttons["Frame"] = dtk::ToolButton::create(context);
            p.buttons["Frame"]->setIcon(p.actions["Frame"]->icon);
            p.buttons["Frame"]->setCheckable(p.actions["Frame"]->checkable);
            p.buttons["Frame"]->setTooltip(p.actions["Frame"]->toolTip);

            p.buttons["Zoom1To1"] = dtk::ToolButton::create(context);
            p.buttons["Zoom1To1"]->setIcon(p.actions["Zoom1To1"]->icon);
            p.buttons["Zoom1To1"]->setTooltip(p.actions["Zoom1To1"]->toolTip);

            p.layout = dtk::HorizontalLayout::create(context, shared_from_this());
            p.layout->setSpacingRole(dtk::SizeRole::None);
            p.buttons["Frame"]->setParent(p.layout);
            p.buttons["Zoom1To1"]->setParent(p.layout);

            auto mainWindowWeak = std::weak_ptr<MainWindow>(mainWindow);
            p.buttons["Frame"]->setCheckedCallback(
                [mainWindowWeak](bool value)
                {
                    if (auto mainWindow = mainWindowWeak.lock())
                    {
                        mainWindow->getViewport()->setFrameView(value);
                    }
                });

            p.buttons["Zoom1To1"]->setClickedCallback(
                [mainWindowWeak]
                {
                    if (auto mainWindow = mainWindowWeak.lock())
                    {
                        mainWindow->getViewport()->viewZoom1To1();
                    }
                });

            p.frameViewObserver = dtk::ValueObserver<bool>::create(
                mainWindow->getViewport()->observeFrameView(),
                [this](bool value)
                {
                    _p->buttons["Frame"]->setChecked(value);
                });
        }

        ViewToolBar::ViewToolBar() :
            _p(new Private)
        {}

        ViewToolBar::~ViewToolBar()
        {}

        std::shared_ptr<ViewToolBar> ViewToolBar::create(
            const std::shared_ptr<dtk::Context>& context,
            const std::shared_ptr<App>& app,
            const std::shared_ptr<MainWindow>& mainWindow,
            const std::map<std::string, std::shared_ptr<dtk::Action> >& actions,
            const std::shared_ptr<IWidget>& parent)
        {
            auto out = std::shared_ptr<ViewToolBar>(new ViewToolBar);
            out->_init(context, app, mainWindow, actions, parent);
            return out;
        }

        void ViewToolBar::setGeometry(const dtk::Box2I& value)
        {
            IWidget::setGeometry(value);
            _p->layout->setGeometry(value);
        }

        void ViewToolBar::sizeHintEvent(const dtk::SizeHintEvent& event)
        {
            IWidget::sizeHintEvent(event);
            _setSizeHint(_p->layout->getSizeHint());
        }
    }
}
