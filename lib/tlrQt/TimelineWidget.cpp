// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021 Darby Johnston
// All rights reserved.

#include <tlrQt/TimelineWidget.h>

#include <QFontDatabase>
#include <QGridLayout>
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
            _timeActionButtonGroup = new QButtonGroup(this);
            _timeActionButtonGroup->addButton(_timeActionButtons["Start"]);
            _timeActionButtonGroup->addButton(_timeActionButtons["End"]);
            _timeActionButtonGroup->addButton(_timeActionButtons["FramePrev"]);
            _timeActionButtonGroup->addButton(_timeActionButtons["FrameNext"]);
            _buttonToTimeAction[_timeActionButtons["Start"]] = timeline::TimeAction::Start;
            _buttonToTimeAction[_timeActionButtons["End"]] = timeline::TimeAction::End;
            _buttonToTimeAction[_timeActionButtons["FramePrev"]] = timeline::TimeAction::FramePrev;
            _buttonToTimeAction[_timeActionButtons["FrameNext"]] = timeline::TimeAction::FrameNext;

            _speedLabel = new SpeedLabel;
            _speedLabel->setToolTip(tr("Timeline speed (frames per second)"));

            _timelineSlider = new TimelineSlider;
            _timelineSlider->setToolTip(tr("Timeline slider"));

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

            auto layout = new QGridLayout;
            layout->setMargin(5);
            layout->setSpacing(5);
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
            layout->addLayout(hLayout, 0, 0);
            hLayout = new QHBoxLayout;
            hLayout->setMargin(0);
            hLayout->addWidget(_speedLabel);
            layout->addLayout(hLayout, 1, 0);
            layout->addWidget(_timelineSlider, 0, 1);
            hLayout = new QHBoxLayout;
            hLayout->setMargin(0);
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
            layout->addLayout(hLayout, 1, 1);
            layout->setColumnStretch(1, 1);
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
            _timelineSlider->setTimeObject(timeObject);
            _currentTimeSpinBox->setTimeObject(timeObject);
            _inPointSpinBox->setTimeObject(timeObject);
            _outPointSpinBox->setTimeObject(timeObject);
            _durationLabel->setTimeObject(timeObject);
        }

        void TimelineWidget::setTimeline(TimelineObject* timeline)
        {
            if (timeline == _timeline)
                return;
            if (_timeline)
            {
                disconnect(
                    _timeline,
                    SIGNAL(playbackChanged(tlr::timeline::Playback)),
                    this,
                    SLOT(_playbackCallback(tlr::timeline::Playback)));
                disconnect(
                    _timeline,
                    SIGNAL(currentTimeChanged(const otime::RationalTime&)),
                    this,
                    SLOT(_currentTimeCallback2(const otime::RationalTime&)));
                disconnect(
                    _timeline,
                    SIGNAL(inOutRangeChanged(const otime::TimeRange&)),
                    this,
                    SLOT(_inOutRangeCallback(const otime::TimeRange&)));
            }
            _timeline = timeline;
            _timelineSlider->setTimeline(timeline);
            if (_timeline)
            {
                connect(
                    _timeline,
                    SIGNAL(playbackChanged(tlr::timeline::Playback)),
                    SLOT(_playbackCallback(tlr::timeline::Playback)));
                connect(
                    _timeline,
                    SIGNAL(currentTimeChanged(const otime::RationalTime&)),
                    SLOT(_currentTimeCallback2(const otime::RationalTime&)));
                connect(
                    _timeline,
                    SIGNAL(inOutRangeChanged(const otime::TimeRange&)),
                    SLOT(_inOutRangeCallback(const otime::TimeRange&)));
            }
            _timelineUpdate();
        }

        void TimelineWidget::_playbackCallback(QAbstractButton* button)
        {
            if (_timeline)
            {
                const auto i = _buttonToPlayback.find(button);
                if (i != _buttonToPlayback.end())
                {
                    _timeline->setPlayback(i.value());
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
            if (_timeline)
            {
                const auto i = _buttonToTimeAction.find(button);
                if (i != _buttonToTimeAction.end())
                {
                    _timeline->timeAction(i.value());
                }
            }
        }

        void TimelineWidget::_currentTimeCallback(const otime::RationalTime& value)
        {
            if (_timeline)
            {
                _timeline->setPlayback(timeline::Playback::Stop);
                _timeline->seek(value);
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
            if (_timeline)
            {
                _timeline->setInOutRange(otime::TimeRange::range_from_start_end_time_inclusive(
                    value,
                    _timeline->inOutRange().end_time_inclusive()));
            }
        }

        void TimelineWidget::_inPointCallback()
        {
            if (_timeline)
            {
                _timeline->setInPoint();
            }
        }

        void TimelineWidget::_resetInPointCallback()
        {
            if (_timeline)
            {
                _timeline->resetInPoint();
            }
        }

        void TimelineWidget::_outPointCallback(const otime::RationalTime& value)
        {
            if (_timeline)
            {
                _timeline->setInOutRange(otime::TimeRange::range_from_start_end_time_inclusive(
                    _timeline->inOutRange().start_time(),
                    value));
            }
        }

        void TimelineWidget::_outPointCallback()
        {
            if (_timeline)
            {
                _timeline->setOutPoint();
            }
        }

        void TimelineWidget::_resetOutPointCallback()
        {
            if (_timeline)
            {
                _timeline->resetOutPoint();
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
            if (_timeline)
            {
                playback = _timeline->playback();
            }
            _playbackToButton[playback]->setChecked(true);
        }

        void TimelineWidget::_timelineUpdate()
        {
            if (_timeline)
            {
                {
                    const QSignalBlocker blocker(_currentTimeSpinBox);
                    _playbackToButton[_timeline->playback()]->setChecked(true);
                }
                for (const auto& button : _playbackButtons)
                {
                    button->setEnabled(true);
                }

                for (const auto& button : _timeActionButtons)
                {
                    button->setEnabled(true);
                }

                const auto& duration = _timeline->duration();
                _speedLabel->setValue(duration);

                _timelineSlider->setEnabled(true);

                {
                    const QSignalBlocker blocker(_currentTimeSpinBox);
                    _currentTimeSpinBox->setValue(_timeline->currentTime());
                }
                _currentTimeSpinBox->setEnabled(true);

                {
                    const QSignalBlocker blocker(_inPointSpinBox);
                    _inPointSpinBox->setValue(_timeline->inOutRange().start_time());
                }
                _inPointSpinBox->setEnabled(true);
                {
                    const QSignalBlocker blocker(_outPointSpinBox);
                    _outPointSpinBox->setValue(_timeline->inOutRange().end_time_inclusive());
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

                _timelineSlider->setEnabled(false);

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
