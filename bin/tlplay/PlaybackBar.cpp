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
            const std::shared_ptr<feather_tk::Context>& context,
            const std::shared_ptr<App>& app,
            const std::map<std::string, std::shared_ptr<feather_tk::Action> >& actions,
            const std::shared_ptr<IWidget>& parent)
        {
            IWidget::_init(context, "PlaybackBar", parent);

            _layout = feather_tk::HorizontalLayout::create(context, shared_from_this());
            _layout->setMarginRole(feather_tk::SizeRole::MarginInside);

            auto hLayout = feather_tk::HorizontalLayout::create(context, _layout);
            hLayout->setSpacingRole(feather_tk::SizeRole::SpacingTool);
            auto tmp = actions;
            auto reverseButton = feather_tk::ToolButton::create(context, tmp["Reverse"], hLayout);
            auto stopButton = feather_tk::ToolButton::create(context, tmp["Stop"], hLayout);
            auto forwardButton = feather_tk::ToolButton::create(context, tmp["Forward"], hLayout);

            hLayout = feather_tk::HorizontalLayout::create(context, _layout);
            hLayout->setSpacingRole(feather_tk::SizeRole::SpacingTool);
            auto startButton = feather_tk::ToolButton::create(context, tmp["Start"], hLayout);
            auto prevButton = feather_tk::ToolButton::create(context, tmp["Prev"], hLayout);
            prevButton->setRepeatClick(true);
            auto nextButton = feather_tk::ToolButton::create(context, tmp["Next"], hLayout);
            nextButton->setRepeatClick(true);
            auto endButton = feather_tk::ToolButton::create(context, tmp["End"], hLayout);

            _currentTimeEdit = timelineui::TimeEdit::create(context, app->getTimeUnitsModel(), _layout);
            _currentTimeEdit->setTooltip("The current time.");

            _durationLabel = timelineui::TimeLabel::create(context, app->getTimeUnitsModel(), _layout);
            _durationLabel->setTooltip("The timeline duration.");

            _timeUnitsComboBox = feather_tk::ComboBox::create(
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

            _playerObserver = feather_tk::ValueObserver<std::shared_ptr<timeline::Player> >::create(
                app->getFilesModel()->observePlayer(),
                [this](const std::shared_ptr<timeline::Player>& value)
                {
                    _player = value;

                    if (value)
                    {
                        _durationLabel->setValue(value->getTimeRange().duration());

                        _currentTimeObserver = feather_tk::ValueObserver<OTIO_NS::RationalTime>::create(
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

            _timeUnitsObserver = feather_tk::ValueObserver<timeline::TimeUnits>::create(
                app->getTimeUnitsModel()->observeTimeUnits(),
                [this](timeline::TimeUnits value)
                {
                    _timeUnitsComboBox->setCurrentIndex(static_cast<int>(value));
                });
        }

        PlaybackBar::~PlaybackBar()
        {}

        std::shared_ptr<PlaybackBar> PlaybackBar::create(
            const std::shared_ptr<feather_tk::Context>& context,
            const std::shared_ptr<App>& app,
            const std::map<std::string, std::shared_ptr<feather_tk::Action> >& actions,
            const std::shared_ptr<IWidget>& parent)
        {
            auto out = std::shared_ptr<PlaybackBar>(new PlaybackBar);
            out->_init(context, app, actions, parent);
            return out;
        }

        void PlaybackBar::setGeometry(const feather_tk::Box2I& value)
        {
            IWidget::setGeometry(value);
            _layout->setGeometry(value);
        }

        void PlaybackBar::sizeHintEvent(const feather_tk::SizeHintEvent& event)
        {
            IWidget::sizeHintEvent(event);
            _setSizeHint(_layout->getSizeHint());
        }
    }
}