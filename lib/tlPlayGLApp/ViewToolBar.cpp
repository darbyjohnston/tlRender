// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2023 Darby Johnston
// All rights reserved.

#include <tlPlayGLApp/ViewToolBar.h>

#include <tlPlayGLApp/App.h>
#include <tlPlayGLApp/MainWindow.h>

#include <tlTimelineUI/TimelineViewport.h>

#include <tlUI/RowLayout.h>
#include <tlUI/ToolButton.h>

namespace tl
{
    namespace play_gl
    {
        struct ViewToolBar::Private
        {
            std::weak_ptr<App> app;
            std::shared_ptr<timeline::Player> player;

            std::map<std::string, std::shared_ptr<ui::ToolButton> > buttons;
            std::shared_ptr<ui::HorizontalLayout> layout;

            std::shared_ptr<observer::ListObserver<std::shared_ptr<timeline::Player> > > playerObserver;
            std::shared_ptr<observer::ValueObserver<bool> > frameViewObserver;
        };

        void ViewToolBar::_init(
            const std::shared_ptr<MainWindow>& mainWindow,
            const std::shared_ptr<App>& app,
            const std::shared_ptr<system::Context>& context)
        {
            IWidget::_init("tl::examples::play_gl::ViewToolBar", context);
            TLRENDER_P();

            p.app = app;

            p.buttons["Frame"] = ui::ToolButton::create(context);
            p.buttons["Frame"]->setIcon("ViewFrame");
            p.buttons["Frame"]->setCheckable(true);

            p.buttons["Zoom1To1"] = ui::ToolButton::create(context);
            p.buttons["Zoom1To1"]->setIcon("ViewZoom1To1");

            p.layout = ui::HorizontalLayout::create(context, shared_from_this());
            p.layout->setSpacingRole(ui::SizeRole::None);
            p.buttons["Frame"]->setParent(p.layout);
            p.buttons["Zoom1To1"]->setParent(p.layout);

            auto mainWindowWeak = std::weak_ptr<MainWindow>(mainWindow);
            p.buttons["Frame"]->setCheckedCallback(
                [mainWindowWeak](bool value)
                {
                    if (auto mainWindow = mainWindowWeak.lock())
                    {
                        mainWindow->getTimelineViewport()->setFrameView(value);
                    }
                });

            p.buttons["Zoom1To1"]->setClickedCallback(
                [mainWindowWeak]
                {
                    if (auto mainWindow = mainWindowWeak.lock())
                    {
                        mainWindow->getTimelineViewport()->viewZoom1To1();
                    }
                });

            p.playerObserver = observer::ListObserver<std::shared_ptr<timeline::Player> >::create(
                app->observeActivePlayers(),
                [this](const std::vector<std::shared_ptr<timeline::Player> >& value)
                {
                    _p->player = !value.empty() ? value[0] : nullptr;
                });

            p.frameViewObserver = observer::ValueObserver<bool>::create(
                mainWindow->getTimelineViewport()->observeFrameView(),
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
            const std::shared_ptr<MainWindow>& mainWindow,
            const std::shared_ptr<App>& app,
            const std::shared_ptr<system::Context>& context)
        {
            auto out = std::shared_ptr<ViewToolBar>(new ViewToolBar);
            out->_init(mainWindow, app, context);
            return out;
        }

        void ViewToolBar::setGeometry(const math::BBox2i& value)
        {
            IWidget::setGeometry(value);
            _p->layout->setGeometry(value);
        }

        void ViewToolBar::sizeHintEvent(const ui::SizeHintEvent& event)
        {
            IWidget::sizeHintEvent(event);
            _sizeHint = _p->layout->getSizeHint();
        }
    }
}