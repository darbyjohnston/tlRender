// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2024 Darby Johnston
// All rights reserved.

#include "PlaybackBar.h"

#include <dtk/ui/ToolButton.h>

namespace tl
{
    namespace examples
    {
        namespace player
        {
            void PlaybackBar::_init(
                const std::shared_ptr<dtk::Context>& context,
                const std::shared_ptr<App>& app,
                const std::map<std::string, std::shared_ptr<dtk::Action> >& actions,
                const std::shared_ptr<IWidget>& parent)
            {
                IWidget::_init(context, "PlaybackBar", parent);

                _layout = dtk::HorizontalLayout::create(context, shared_from_this());

                auto hLayout = dtk::HorizontalLayout::create(context, _layout);
                hLayout->setSpacingRole(dtk::SizeRole::SpacingTool);
                auto tmp = actions;
                auto reverseButton = dtk::ToolButton::create(context, tmp["Reverse"], hLayout);
                auto stopButton = dtk::ToolButton::create(context, tmp["Stop"], hLayout);
                auto forwardButton = dtk::ToolButton::create(context, tmp["Forward"], hLayout);
            }

            PlaybackBar::~PlaybackBar()
            {}

            std::shared_ptr<PlaybackBar> PlaybackBar::create(
                const std::shared_ptr<dtk::Context>& context,
                const std::shared_ptr<App>& app,
                const std::map<std::string, std::shared_ptr<dtk::Action> >& actions,
                const std::shared_ptr<IWidget>& parent)
            {
                auto out = std::shared_ptr<PlaybackBar>(new PlaybackBar);
                out->_init(context, app, actions, parent);
                return out;
            }

            void PlaybackBar::setGeometry(const dtk::Box2I& value)
            {
                IWidget::setGeometry(value);
                _layout->setGeometry(value);
            }

            void PlaybackBar::sizeHintEvent(const dtk::SizeHintEvent& event)
            {
                IWidget::sizeHintEvent(event);
                _setSizeHint(_layout->getSizeHint());
            }
        }
    }
}
