// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2024 Darby Johnston
// All rights reserved.

#include "PlaybackBar.h"

namespace tl
{
    namespace examples
    {
        namespace player
        {
            void PlaybackBar::_init(
                const std::shared_ptr<dtk::Context>& context,
                const std::shared_ptr<App>& app)
            {
                IWidget::_init(context, "PlaybackBar", nullptr);
                _layout = dtk::HorizontalLayout::create(context, shared_from_this());
            }

            PlaybackBar::~PlaybackBar()
            {}

            std::shared_ptr<PlaybackBar> PlaybackBar::create(
                const std::shared_ptr<dtk::Context>& context,
                const std::shared_ptr<App>& app)
            {
                auto out = std::shared_ptr<PlaybackBar>(new PlaybackBar);
                out->_init(context, app);
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
