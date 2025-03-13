// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2025 Darby Johnston
// All rights reserved.

#include <tlPlayApp/Widgets/ViewToolBar.h>

#include <tlPlayApp/Actions/ViewActions.h>

#include <dtk/ui/RowLayout.h>
#include <dtk/ui/ToolButton.h>

namespace tl
{
    namespace play
    {
        struct ViewToolBar::Private
        {
            std::map<std::string, std::shared_ptr<dtk::ToolButton> > buttons;
            std::shared_ptr<dtk::HorizontalLayout> layout;
        };

        void ViewToolBar::_init(
            const std::shared_ptr<dtk::Context>& context,
            const std::shared_ptr<ViewActions>& viewActions,
            const std::shared_ptr<IWidget>& parent)
        {
            IWidget::_init(
                context,
                "tl::play_app::ViewToolBar",
                parent);
            DTK_P();

            auto actions = viewActions->getActions();
            p.buttons["Frame"] = dtk::ToolButton::create(context, actions["Frame"]);
            p.buttons["ZoomReset"] = dtk::ToolButton::create(context, actions["ZoomReset"]);

            p.layout = dtk::HorizontalLayout::create(context, shared_from_this());
            p.layout->setSpacingRole(dtk::SizeRole::None);
            p.buttons["Frame"]->setParent(p.layout);
            p.buttons["ZoomReset"]->setParent(p.layout);
        }

        ViewToolBar::ViewToolBar() :
            _p(new Private)
        {}

        ViewToolBar::~ViewToolBar()
        {}

        std::shared_ptr<ViewToolBar> ViewToolBar::create(
            const std::shared_ptr<dtk::Context>& context,
            const std::shared_ptr<ViewActions>& viewActions,
            const std::shared_ptr<IWidget>& parent)
        {
            auto out = std::shared_ptr<ViewToolBar>(new ViewToolBar);
            out->_init(context, viewActions, parent);
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
