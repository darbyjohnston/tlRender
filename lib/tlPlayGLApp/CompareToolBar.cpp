// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2023 Darby Johnston
// All rights reserved.

#include <tlPlayGLApp/CompareToolBar.h>

#include <tlPlayGLApp/App.h>

#include <tlUI/RowLayout.h>
#include <tlUI/ToolButton.h>

namespace tl
{
    namespace play_gl
    {
        struct CompareToolBar::Private
        {
            std::weak_ptr<App> app;
            std::shared_ptr<timeline::Player> player;

            std::map<std::string, std::shared_ptr<ui::ToolButton> > buttons;
            std::shared_ptr<ui::HorizontalLayout> layout;

            std::shared_ptr<observer::ValueObserver<std::shared_ptr<timeline::Player> > > playerObserver;
        };

        void CompareToolBar::_init(
            const std::shared_ptr<App>& app,
            const std::shared_ptr<system::Context>& context)
        {
            IWidget::_init("tl::examples::play_gl::CompareToolBar", context);
            TLRENDER_P();

            p.app = app;

            p.buttons["A"] = ui::ToolButton::create(context);
            p.buttons["A"]->setIcon("CompareA");
            p.buttons["A"]->setEnabled(false);

            p.buttons["B"] = ui::ToolButton::create(context);
            p.buttons["B"]->setIcon("CompareB");
            p.buttons["B"]->setEnabled(false);

            p.buttons["Wipe"] = ui::ToolButton::create(context);
            p.buttons["Wipe"]->setIcon("CompareWipe");
            p.buttons["Wipe"]->setEnabled(false);

            p.buttons["Overlay"] = ui::ToolButton::create(context);
            p.buttons["Overlay"]->setIcon("CompareOverlay");
            p.buttons["Overlay"]->setEnabled(false);

            p.buttons["Difference"] = ui::ToolButton::create(context);
            p.buttons["Difference"]->setIcon("CompareDifference");
            p.buttons["Difference"]->setEnabled(false);

            p.buttons["Horizontal"] = ui::ToolButton::create(context);
            p.buttons["Horizontal"]->setIcon("CompareHorizontal");
            p.buttons["Horizontal"]->setEnabled(false);

            p.buttons["Vertical"] = ui::ToolButton::create(context);
            p.buttons["Vertical"]->setIcon("CompareVertical");
            p.buttons["Vertical"]->setEnabled(false);

            p.buttons["Tile"] = ui::ToolButton::create(context);
            p.buttons["Tile"]->setIcon("CompareTile");
            p.buttons["Tile"]->setEnabled(false);

            p.layout = ui::HorizontalLayout::create(context, shared_from_this());
            p.layout->setSpacingRole(ui::SizeRole::None);
            p.buttons["A"]->setParent(p.layout);
            p.buttons["B"]->setParent(p.layout);
            p.buttons["Wipe"]->setParent(p.layout);
            p.buttons["Overlay"]->setParent(p.layout);
            p.buttons["Difference"]->setParent(p.layout);
            p.buttons["Horizontal"]->setParent(p.layout);
            p.buttons["Vertical"]->setParent(p.layout);
            p.buttons["Tile"]->setParent(p.layout);

            auto appWeak = std::weak_ptr<App>(app);

            p.playerObserver = observer::ValueObserver<std::shared_ptr<timeline::Player> >::create(
                app->observePlayer(),
                [this](const std::shared_ptr<timeline::Player>& value)
                {
                    _p->player = value;
                });
        }

        CompareToolBar::CompareToolBar() :
            _p(new Private)
        {}

        CompareToolBar::~CompareToolBar()
        {}

        std::shared_ptr<CompareToolBar> CompareToolBar::create(
            const std::shared_ptr<App>& app,
            const std::shared_ptr<system::Context>& context)
        {
            auto out = std::shared_ptr<CompareToolBar>(new CompareToolBar);
            out->_init(app, context);
            return out;
        }

        void CompareToolBar::setGeometry(const math::BBox2i& value)
        {
            IWidget::setGeometry(value);
            _p->layout->setGeometry(value);
        }

        void CompareToolBar::sizeHintEvent(const ui::SizeHintEvent& event)
        {
            IWidget::sizeHintEvent(event);
            _sizeHint = _p->layout->getSizeHint();
        }
    }
}
