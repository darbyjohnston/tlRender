// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2024 Darby Johnston
// All rights reserved.

#include "PlaybackBar.h"

#include "App.h"
#include "FilesModel.h"

#include <feather-tk/ui/ToolButton.h>

namespace tl
{
    namespace play
    {
        void PlaybackBar::_init(
            const std::shared_ptr<ftk::Context>& context,
            const std::shared_ptr<App>& app,
            const std::map<std::string, std::shared_ptr<ftk::Action> >& actions,
            const std::shared_ptr<IWidget>& parent)
        {
            IWidget::_init(context, "PlaybackBar", parent);

            _layout = ftk::HorizontalLayout::create(context, shared_from_this());
            _layout->setMarginRole(ftk::SizeRole::MarginInside);

            auto hLayout = ftk::HorizontalLayout::create(context, _layout);
            hLayout->setSpacingRole(ftk::SizeRole::SpacingTool);
            auto tmp = actions;
            auto reverseButton = ftk::ToolButton::create(context, tmp["Reverse"], hLayout);
            auto stopButton = ftk::ToolButton::create(context, tmp["Stop"], hLayout);
            auto forwardButton = ftk::ToolButton::create(context, tmp["Forward"], hLayout);

            hLayout = ftk::HorizontalLayout::create(context, _layout);
            hLayout->setSpacingRole(ftk::SizeRole::SpacingTool);
            auto startButton = ftk::ToolButton::create(context, tmp["Start"], hLayout);
            auto prevButton = ftk::ToolButton::create(context, tmp["Prev"], hLayout);
            prevButton->setRepeatClick(true);
            auto nextButton = ftk::ToolButton::create(context, tmp["Next"], hLayout);
            nextButton->setRepeatClick(true);
            auto endButton = ftk::ToolButton::create(context, tmp["End"], hLayout);

            _currentTimeEdit = timelineui::TimeEdit::create(context, app->getTimeUnitsModel(), _layout);
            _currentTimeEdit->setTooltip("The current time.");

            _durationLabel = timelineui::TimeLabel::create(context, app->getTimeUnitsModel(), _layout);
            _durationLabel->setTooltip("The timeline duration.");

            _timeUnitsComboBox = ftk::ComboBox::create(
                context,
                timeline::getTimeUnitsLabels(),
                _layout);

            _currentTimeEdit->setCallback(
                [this](const OTIO_NS::RationalTime& value)
                {
                    if (_player)
                    {
                        _player->stop();
                        _player->seek(value);
                    }
                });

            std::weak_ptr<App> appWeak(app);
            _timeUnitsComboBox->setIndexCallback(
                [appWeak](int value)
                {
                    if (auto app = appWeak.lock())
                    {
                        app->getTimeUnitsModel()->setTimeUnits(
                            static_cast<timeline::TimeUnits>(value));
                    }
                });

            _playerObserver = ftk::ValueObserver<std::shared_ptr<timeline::Player> >::create(
                app->getFilesModel()->observePlayer(),
                [this](const std::shared_ptr<timeline::Player>& value)
                {
                    _player = value;

                    if (value)
                    {
                        _durationLabel->setValue(value->getTimeRange().duration());

                        _currentTimeObserver = ftk::ValueObserver<OTIO_NS::RationalTime>::create(
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

            _timeUnitsObserver = ftk::ValueObserver<timeline::TimeUnits>::create(
                app->getTimeUnitsModel()->observeTimeUnits(),
                [this](timeline::TimeUnits value)
                {
                    _timeUnitsComboBox->setCurrentIndex(static_cast<int>(value));
                });
        }

        PlaybackBar::~PlaybackBar()
        {}

        std::shared_ptr<PlaybackBar> PlaybackBar::create(
            const std::shared_ptr<ftk::Context>& context,
            const std::shared_ptr<App>& app,
            const std::map<std::string, std::shared_ptr<ftk::Action> >& actions,
            const std::shared_ptr<IWidget>& parent)
        {
            auto out = std::shared_ptr<PlaybackBar>(new PlaybackBar);
            out->_init(context, app, actions, parent);
            return out;
        }

        void PlaybackBar::setGeometry(const ftk::Box2I& value)
        {
            IWidget::setGeometry(value);
            _layout->setGeometry(value);
        }

        void PlaybackBar::sizeHintEvent(const ftk::SizeHintEvent& event)
        {
            IWidget::sizeHintEvent(event);
            _setSizeHint(_layout->getSizeHint());
        }
    }
}