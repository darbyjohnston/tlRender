// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021 Darby Johnston
// All rights reserved.

#include <tlrQt/TimelineWidget.h>

#include <QFontDatabase>
#include <QHBoxLayout>
#include <QStyle>
#include <QVBoxLayout>

namespace tlr
{
    namespace qt
    {
        TimelineWidget::TimelineWidget(QWidget* parent) :
            QWidget(parent)
        {
            _viewport = new TimelineViewport;

            _playbackButtons["Stop"] = new QToolButton;
            _playbackButtons["Stop"]->setCheckable(true);
            _playbackButtons["Stop"]->setIcon(QIcon(":/Icons/PlaybackStop.svg"));
            _playbackButtons["Stop"]->setToolTip(tr("Stop playback"));
            _playbackButtons["Forward"] = new QToolButton;
            _playbackButtons["Forward"]->setCheckable(true);
            _playbackButtons["Forward"]->setIcon(QIcon(":/Icons/PlaybackForward.svg"));
            _playbackButtons["Forward"]->setToolTip(tr("Forward playback"));
            _playbackButtons["Reverse"] = new QToolButton;
            _playbackButtons["Reverse"]->setCheckable(true);
            _playbackButtons["Reverse"]->setIcon(QIcon(":/Icons/PlaybackReverse.svg"));
            _playbackButtons["Reverse"]->setToolTip(tr("Reverse playback"));
            _playbackButtonGroup = new QButtonGroup(this);
            _playbackButtonGroup->setExclusive(true);
            _playbackButtonGroup->addButton(_playbackButtons["Stop"]);
            _playbackButtonGroup->addButton(_playbackButtons["Forward"]);
            _playbackButtonGroup->addButton(_playbackButtons["Reverse"]);
            _buttonToPlayback[_playbackButtons["Stop"]] = timeline::Playback::Stop;
            _buttonToPlayback[_playbackButtons["Forward"]] = timeline::Playback::Forward;
            _buttonToPlayback[_playbackButtons["Reverse"]] = timeline::Playback::Reverse;
            _playbackToButton[timeline::Playback::Stop] = _playbackButtons["Stop"];
            _playbackToButton[timeline::Playback::Forward] = _playbackButtons["Forward"];
            _playbackToButton[timeline::Playback::Reverse] = _playbackButtons["Reverse"];

            _timeActionButtons["Start"] = new QToolButton;
            _timeActionButtons["Start"]->setIcon(QIcon(":/Icons/TimeStart.svg"));
            _timeActionButtons["Start"]->setToolTip(tr("Go to the start time"));
            _timeActionButtons["End"] = new QToolButton;
            _timeActionButtons["End"]->setIcon(QIcon(":/Icons/TimeEnd.svg"));
            _timeActionButtons["End"]->setToolTip(tr("Go to the end time"));
            _timeActionButtons["FramePrev"] = new QToolButton;
            _timeActionButtons["FramePrev"]->setAutoRepeat(true);
            _timeActionButtons["FramePrev"]->setIcon(QIcon(":/Icons/FramePrev.svg"));
            _timeActionButtons["FramePrev"]->setToolTip(tr("Go to the previous frame"));
            _timeActionButtons["FrameNext"] = new QToolButton;
            _timeActionButtons["FrameNext"]->setAutoRepeat(true);
            _timeActionButtons["FrameNext"]->setIcon(QIcon(":/Icons/FrameNext.svg"));
            _timeActionButtons["FrameNext"]->setToolTip(tr("Go to the next frame"));
            _timeActionButtons["ClipPrev"] = new QToolButton;
            _timeActionButtons["ClipPrev"]->setAutoRepeat(true);
            _timeActionButtons["ClipPrev"]->setIcon(QIcon(":/Icons/ClipPrev.svg"));
            _timeActionButtons["ClipPrev"]->setToolTip(tr("Go to the previous clip"));
            _timeActionButtons["ClipNext"] = new QToolButton;
            _timeActionButtons["ClipNext"]->setAutoRepeat(true);
            _timeActionButtons["ClipNext"]->setIcon(QIcon(":/Icons/ClipNext.svg"));
            _timeActionButtons["ClipNext"]->setToolTip(tr("Go to the next clip"));
            _timeActionButtonGroup = new QButtonGroup(this);
            _timeActionButtonGroup->addButton(_timeActionButtons["Start"]);
            _timeActionButtonGroup->addButton(_timeActionButtons["End"]);
            _timeActionButtonGroup->addButton(_timeActionButtons["FramePrev"]);
            _timeActionButtonGroup->addButton(_timeActionButtons["FrameNext"]);
            _timeActionButtonGroup->addButton(_timeActionButtons["ClipPrev"]);
            _timeActionButtonGroup->addButton(_timeActionButtons["ClipNext"]);
            _buttonToTimeAction[_timeActionButtons["Start"]] = timeline::TimeAction::Start;
            _buttonToTimeAction[_timeActionButtons["End"]] = timeline::TimeAction::End;
            _buttonToTimeAction[_timeActionButtons["FramePrev"]] = timeline::TimeAction::FramePrev;
            _buttonToTimeAction[_timeActionButtons["FrameNext"]] = timeline::TimeAction::FrameNext;
            _buttonToTimeAction[_timeActionButtons["ClipPrev"]] = timeline::TimeAction::ClipPrev;
            _buttonToTimeAction[_timeActionButtons["ClipNext"]] = timeline::TimeAction::ClipNext;

            _speedLabel = new SpeedLabel;
            _speedLabel->setToolTip(tr("Timeline speed (frames per second)"));

            _slider = new TimelineSlider;
            _slider->setToolTip(tr("Timeline slider"));

            _currentTimeSpinBox = new TimeSpinBox;
            _currentTimeSpinBox->setToolTip(tr("Current time"));

            _inPointSpinBox = new TimeSpinBox;
            _inPointSpinBox->setToolTip(tr("Playback in point"));

            _outPointSpinBox = new TimeSpinBox;
            _outPointSpinBox->setToolTip(tr("Playback out point"));

            _inOutButtons["SetInPoint"] = new QToolButton;
            _inOutButtons["SetInPoint"]->setIcon(QIcon(":/Icons/TimeStart.svg"));
            _inOutButtons["SetInPoint"]->setToolTip(tr("Set the playback in point to the current frame"));
            _inOutButtons["ResetInPoint"] = new QToolButton;
            _inOutButtons["ResetInPoint"]->setIcon(QIcon(":/Icons/Reset.svg"));
            _inOutButtons["ResetInPoint"]->setToolTip(tr("Reset the playback in point"));
            _inOutButtons["SetOutPoint"] = new QToolButton;
            _inOutButtons["SetOutPoint"]->setIcon(QIcon(":/Icons/TimeEnd.svg"));
            _inOutButtons["SetOutPoint"]->setToolTip(tr("Set the playback out point to the current frame"));
            _inOutButtons["ResetOutPoint"] = new QToolButton;
            _inOutButtons["ResetOutPoint"]->setIcon(QIcon(":/Icons/Reset.svg"));
            _inOutButtons["ResetOutPoint"]->setToolTip(tr("Reset the playback out point"));

            _durationLabel = new TimeLabel;
            _durationLabel->setToolTip(tr("Timeline duration"));

            auto layout = new QVBoxLayout;
            layout->setMargin(0);
            layout->setSpacing(0);
            layout->addWidget(_viewport, 1);
            auto vLayout = new QVBoxLayout;
            vLayout->setMargin(5);
            vLayout->setSpacing(5);
            vLayout->addWidget(_slider, 1);
            auto hLayout = new QHBoxLayout;
            hLayout->setMargin(0);
            auto hLayout2 = new QHBoxLayout;
            hLayout2->setSpacing(1);
            hLayout2->addWidget(_playbackButtons["Reverse"]);
            hLayout2->addWidget(_playbackButtons["Stop"]);
            hLayout2->addWidget(_playbackButtons["Forward"]);
            hLayout->addLayout(hLayout2);
            hLayout2 = new QHBoxLayout;
            hLayout2->setSpacing(1);
            hLayout2->addWidget(_timeActionButtons["Start"]);
            hLayout2->addWidget(_timeActionButtons["FramePrev"]);
            hLayout2->addWidget(_timeActionButtons["FrameNext"]);
            hLayout2->addWidget(_timeActionButtons["End"]);
            hLayout->addLayout(hLayout2);
            hLayout2 = new QHBoxLayout;
            hLayout2->setSpacing(1);
            hLayout2->addWidget(_timeActionButtons["ClipPrev"]);
            hLayout2->addWidget(_timeActionButtons["ClipNext"]);
            hLayout->addLayout(hLayout2);
            hLayout->addWidget(_currentTimeSpinBox);
            hLayout->addWidget(_inPointSpinBox);
            hLayout2 = new QHBoxLayout;
            hLayout2->setSpacing(1);
            hLayout2->addWidget(_inOutButtons["SetInPoint"]);
            hLayout2->addWidget(_inOutButtons["ResetInPoint"]);
            hLayout->addLayout(hLayout2);
            hLayout->addStretch();
            hLayout2 = new QHBoxLayout;
            hLayout2->setSpacing(1);
            hLayout2->addWidget(_inOutButtons["ResetOutPoint"]);
            hLayout2->addWidget(_inOutButtons["SetOutPoint"]);
            hLayout->addLayout(hLayout2);
            hLayout->addWidget(_outPointSpinBox);
            hLayout->addWidget(_durationLabel);
            hLayout->addWidget(_speedLabel);
            vLayout->addLayout(hLayout);
            layout->addLayout(vLayout);
            setLayout(layout);

            _playbackUpdate();
            _timelineUpdate();

            connect(
                _playbackButtonGroup,
                SIGNAL(buttonClicked(QAbstractButton*)),
                SLOT(_playbackCallback(QAbstractButton*)));

            connect(
                _timeActionButtonGroup,
                SIGNAL(buttonClicked(QAbstractButton*)),
                SLOT(_timeActionCallback(QAbstractButton*)));

            connect(
                _currentTimeSpinBox,
                SIGNAL(valueChanged(const otime::RationalTime&)),
                SLOT(_currentTimeCallback(const otime::RationalTime&)));

            connect(
                _inPointSpinBox,
                SIGNAL(valueChanged(const otime::RationalTime&)),
                SLOT(_inPointCallback(const otime::RationalTime&)));
            connect(
                _outPointSpinBox,
                SIGNAL(valueChanged(const otime::RationalTime&)),
                SLOT(_outPointCallback(const otime::RationalTime&)));

            connect(
                _inOutButtons["SetInPoint"],
                SIGNAL(clicked()),
                SLOT(_inPointCallback()));
            connect(
                _inOutButtons["ResetInPoint"],
                SIGNAL(clicked()),
                SLOT(_resetInPointCallback()));
            connect(
                _inOutButtons["SetOutPoint"],
                SIGNAL(clicked()),
                SLOT(_outPointCallback()));
            connect(
                _inOutButtons["ResetOutPoint"],
                SIGNAL(clicked()),
                SLOT(_resetOutPointCallback()));
        }

        void TimelineWidget::setTimeObject(TimeObject* timeObject)
        {
            _slider->setTimeObject(timeObject);
            _currentTimeSpinBox->setTimeObject(timeObject);
            _inPointSpinBox->setTimeObject(timeObject);
            _outPointSpinBox->setTimeObject(timeObject);
            _durationLabel->setTimeObject(timeObject);
        }

        void TimelineWidget::setTimelinePlayer(TimelinePlayer* timelinePlayer)
        {
            if (timelinePlayer == _timelinePlayer)
                return;
            if (_timelinePlayer)
            {
                disconnect(
                    _timelinePlayer,
                    SIGNAL(playbackChanged(tlr::timeline::Playback)),
                    this,
                    SLOT(_playbackCallback(tlr::timeline::Playback)));
                disconnect(
                    _timelinePlayer,
                    SIGNAL(currentTimeChanged(const otime::RationalTime&)),
                    this,
                    SLOT(_currentTimeCallback2(const otime::RationalTime&)));
                disconnect(
                    _timelinePlayer,
                    SIGNAL(inOutRangeChanged(const otime::TimeRange&)),
                    this,
                    SLOT(_inOutRangeCallback(const otime::TimeRange&)));
            }
            _timelinePlayer = timelinePlayer;
            _viewport->setTimelinePlayer(timelinePlayer);
            _slider->setTimelinePlayer(timelinePlayer);
            if (_timelinePlayer)
            {
                connect(
                    _timelinePlayer,
                    SIGNAL(playbackChanged(tlr::timeline::Playback)),
                    SLOT(_playbackCallback(tlr::timeline::Playback)));
                connect(
                    _timelinePlayer,
                    SIGNAL(currentTimeChanged(const otime::RationalTime&)),
                    SLOT(_currentTimeCallback2(const otime::RationalTime&)));
                connect(
                    _timelinePlayer,
                    SIGNAL(inOutRangeChanged(const otime::TimeRange&)),
                    SLOT(_inOutRangeCallback(const otime::TimeRange&)));
            }
            _timelineUpdate();
        }

        void TimelineWidget::_playbackCallback(QAbstractButton* button)
        {
            if (_timelinePlayer)
            {
                const auto i = _buttonToPlayback.find(button);
                if (i != _buttonToPlayback.end())
                {
                    _timelinePlayer->setPlayback(i.value());
                    _playbackUpdate();
                }
            }
        }

        void TimelineWidget::_playbackCallback(tlr::timeline::Playback value)
        {
            _playbackUpdate();
        }

        void TimelineWidget::_timeActionCallback(QAbstractButton* button)
        {
            if (_timelinePlayer)
            {
                const auto i = _buttonToTimeAction.find(button);
                if (i != _buttonToTimeAction.end())
                {
                    _timelinePlayer->timeAction(i.value());
                }
            }
        }

        void TimelineWidget::_currentTimeCallback(const otime::RationalTime& value)
        {
            if (_timelinePlayer)
            {
                _timelinePlayer->setPlayback(timeline::Playback::Stop);
                _timelinePlayer->seek(value);
            }
        }

        void TimelineWidget::_currentTimeCallback2(const otime::RationalTime& value)
        {
            otime::ErrorStatus errorStatus;
            std::string label = value.to_timecode(&errorStatus);
            if (errorStatus != otime::ErrorStatus::OK)
            {
                throw std::runtime_error(errorStatus.details);
            }
            {
                const QSignalBlocker blocker(_currentTimeSpinBox);
                _currentTimeSpinBox->setValue(value);
            }
        }

        void TimelineWidget::_inPointCallback(const otime::RationalTime& value)
        {
            if (_timelinePlayer)
            {
                _timelinePlayer->setInOutRange(otime::TimeRange::range_from_start_end_time_inclusive(
                    value,
                    _timelinePlayer->inOutRange().end_time_inclusive()));
            }
        }

        void TimelineWidget::_inPointCallback()
        {
            if (_timelinePlayer)
            {
                _timelinePlayer->setInPoint();
            }
        }

        void TimelineWidget::_resetInPointCallback()
        {
            if (_timelinePlayer)
            {
                _timelinePlayer->resetInPoint();
            }
        }

        void TimelineWidget::_outPointCallback(const otime::RationalTime& value)
        {
            if (_timelinePlayer)
            {
                _timelinePlayer->setInOutRange(otime::TimeRange::range_from_start_end_time_inclusive(
                    _timelinePlayer->inOutRange().start_time(),
                    value));
            }
        }

        void TimelineWidget::_outPointCallback()
        {
            if (_timelinePlayer)
            {
                _timelinePlayer->setOutPoint();
            }
        }

        void TimelineWidget::_resetOutPointCallback()
        {
            if (_timelinePlayer)
            {
                _timelinePlayer->resetOutPoint();
            }
        }

        void TimelineWidget::_inOutRangeCallback(const otime::TimeRange& value)
        {
            {
                const QSignalBlocker blocker(_inPointSpinBox);
                _inPointSpinBox->setValue(value.start_time());
            }
            {
                const QSignalBlocker blocker(_outPointSpinBox);
                _outPointSpinBox->setValue(value.end_time_inclusive());
            }
        }

        void TimelineWidget::_playbackUpdate()
        {
            timeline::Playback playback = timeline::Playback::Stop;
            if (_timelinePlayer)
            {
                playback = _timelinePlayer->playback();
            }
            _playbackToButton[playback]->setChecked(true);
        }

        void TimelineWidget::_timelineUpdate()
        {
            if (_timelinePlayer)
            {
                {
                    const QSignalBlocker blocker(_currentTimeSpinBox);
                    _playbackToButton[_timelinePlayer->playback()]->setChecked(true);
                }
                for (const auto& button : _playbackButtons)
                {
                    button->setEnabled(true);
                }

                for (const auto& button : _timeActionButtons)
                {
                    button->setEnabled(true);
                }

                const auto& duration = _timelinePlayer->duration();
                _speedLabel->setValue(duration);

                _slider->setEnabled(true);

                {
                    const QSignalBlocker blocker(_currentTimeSpinBox);
                    _currentTimeSpinBox->setValue(_timelinePlayer->currentTime());
                }
                _currentTimeSpinBox->setEnabled(true);

                {
                    const QSignalBlocker blocker(_inPointSpinBox);
                    _inPointSpinBox->setValue(_timelinePlayer->inOutRange().start_time());
                }
                _inPointSpinBox->setEnabled(true);
                {
                    const QSignalBlocker blocker(_outPointSpinBox);
                    _outPointSpinBox->setValue(_timelinePlayer->inOutRange().end_time_inclusive());
                }
                _outPointSpinBox->setEnabled(true);
                for (const auto& button : _inOutButtons)
                {
                    button->setEnabled(true);
                }

                _durationLabel->setValue(duration);
            }
            else
            {
                for (const auto& button : _playbackButtons)
                {
                    button->setChecked(false);
                    button->setEnabled(false);
                }

                for (const auto& button : _timeActionButtons)
                {
                    button->setEnabled(false);
                }

                _speedLabel->setValue(otime::RationalTime());

                _slider->setEnabled(false);

                _currentTimeSpinBox->setValue(otime::RationalTime());
                _currentTimeSpinBox->setEnabled(false);

                _inPointSpinBox->setValue(otime::RationalTime());
                _inPointSpinBox->setEnabled(false);
                _outPointSpinBox->setValue(otime::RationalTime());
                _outPointSpinBox->setEnabled(false);
                for (const auto& button : _inOutButtons)
                {
                    button->setEnabled(false);
                }

                _durationLabel->setValue(otime::RationalTime());
            }
        }
    }
}
