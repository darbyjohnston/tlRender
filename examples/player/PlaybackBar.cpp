// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2024 Darby Johnston
// All rights reserved.

#include "PlaybackBar.h"

#include "App.h"
#include "FilesModel.h"

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

                hLayout = dtk::HorizontalLayout::create(context, _layout);
                hLayout->setSpacingRole(dtk::SizeRole::SpacingTool);
                auto startButton = dtk::ToolButton::create(context, tmp["Start"], hLayout);
                auto prevButton = dtk::ToolButton::create(context, tmp["Prev"], hLayout);
                prevButton->setRepeatClick(true);
                auto nextButton = dtk::ToolButton::create(context, tmp["Next"], hLayout);
                nextButton->setRepeatClick(true);
                auto endButton = dtk::ToolButton::create(context, tmp["End"], hLayout);

                _currentTimeEdit = timelineui::TimeEdit::create(context, app->getTimeUnitsModel(), _layout);
                _currentTimeEdit->setTooltip("The current time.");

                _durationLabel = timelineui::TimeLabel::create(context, app->getTimeUnitsModel(), _layout);
                _durationLabel->setTooltip("The timeline duration.");

                _currentTimeEdit->setCallback(
                    [this](const OTIO_NS::RationalTime& value)
                    {
                        if (_player)
                        {
                            _player->stop();
                            _player->seek(value);
                        }
                    });

                _playerObserver = dtk::ValueObserver<std::shared_ptr<timeline::Player> >::create(
                    app->getFilesModel()->observePlayer(),
                    [this](const std::shared_ptr<timeline::Player>& value)
                    {
                        _player = value;

                        if (value)
                        {
                            _durationLabel->setValue(value->getTimeRange().duration());

                            _currentTimeObserver = dtk::ValueObserver<OTIO_NS::RationalTime>::create(
                                value->observeCurrentTime(),
                                [this](const OTIO_NS::RationalTime& value)
                                {
                                    _currentTimeEdit->setValue(value);
                                });
                        }
                        else
                        {
                            _currentTimeEdit->setValue(time::invalidTime);
                            _durationLabel->setValue(time::invalidTime);

                            _currentTimeObserver.reset();
                        }

                        _currentTimeEdit->setEnabled(value.get());
                        _durationLabel->setEnabled(value.get());
                    });
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
