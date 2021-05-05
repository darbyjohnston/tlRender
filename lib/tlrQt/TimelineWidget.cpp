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
            _playbackButtons[timeline::Playback::Stop] = new QToolButton;
            _playbackButtons[timeline::Playback::Stop]->setCheckable(true);
            _playbackButtons[timeline::Playback::Stop]->setIcon(style()->standardIcon(QStyle::SP_MediaStop));
            _playbackButtons[timeline::Playback::Stop]->setToolTip(tr("Stop playback"));
            _playbackButtons[timeline::Playback::Forward] = new QToolButton;
            _playbackButtons[timeline::Playback::Forward]->setCheckable(true);
            _playbackButtons[timeline::Playback::Forward]->setIcon(style()->standardIcon(QStyle::SP_MediaPlay));
            _playbackButtons[timeline::Playback::Forward]->setToolTip(tr("Forward playback"));

            _viewport = new TimelineViewport;

            _currentTimeSpinBox = new TimeSpinBox;

            _timeSlider = new QSlider(Qt::Orientation::Horizontal);
            _timeSlider->setToolTip(tr("Time slider"));

            _durationLabel = new QLabel;
            const QFont fixedFont = QFontDatabase::systemFont(QFontDatabase::FixedFont);
            _durationLabel->setFont(fixedFont);
            _durationLabel->setText("00:00:00:00");
            _durationLabel->setToolTip(tr("Duration"));

            auto layout = new QVBoxLayout;
            layout->setMargin(0);
            layout->setSpacing(0);
            layout->addWidget(_viewport, 1);
            auto hLayout = new QHBoxLayout;
            hLayout->setMargin(5);
            hLayout->setSpacing(5);
            auto hLayout2 = new QHBoxLayout;
            hLayout2->setMargin(0);
            hLayout2->setSpacing(0);
            hLayout2->addWidget(_playbackButtons[timeline::Playback::Stop]);
            hLayout2->addWidget(_playbackButtons[timeline::Playback::Forward]);
            hLayout->addLayout(hLayout2);
            hLayout->addWidget(_currentTimeSpinBox);
            hLayout->addWidget(_timeSlider);
            hLayout->addWidget(_durationLabel);
            layout->addLayout(hLayout);
            setLayout(layout);

            _playbackUpdate();
            _timelineUpdate();

            connect(
                _playbackButtons[timeline::Playback::Stop],
                SIGNAL(clicked()),
                SLOT(_stopCallback()));
            connect(
                _playbackButtons[timeline::Playback::Forward],
                SIGNAL(clicked()),
                SLOT(_forwardCallback()));
            connect(
                _currentTimeSpinBox,
                SIGNAL(valueChanged(const otime::RationalTime&)),
                SLOT(_currentTimeSpinBoxCallback(const otime::RationalTime&)));
            connect(
                _timeSlider,
                SIGNAL(valueChanged(int)),
                SLOT(_timeSliderCallback(int)));
        }

        void TimelineWidget::setTimeline(TimelineObject* timeline)
        {
            if (timeline == _timeline)
                return;
            _timeline = timeline;
            _viewport->setTimeline(timeline);
            _timelineUpdate();
        }

        void TimelineWidget::_currentTimeCallback(const otime::RationalTime& value)
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
            {
                const QSignalBlocker blocker(_timeSlider);
                _timeSlider->setValue(value.value());
            }
        }

        void TimelineWidget::_playbackCallback(tlr::timeline::Playback value)
        {
            _playbackUpdate();
        }

        void TimelineWidget::_loopCallback(tlr::timeline::Loop value)
        {

        }

        void TimelineWidget::_stopCallback()
        {
            if (_timeline)
            {
                _timeline->stop();
                _playbackUpdate();
            }
        }

        void TimelineWidget::_forwardCallback()
        {
            if (_timeline)
            {
                _timeline->forward();
                _playbackUpdate();
            }
        }

        void TimelineWidget::_currentTimeSpinBoxCallback(const otime::RationalTime& value)
        {
            if (_timeline)
            {
                _timeline->setPlayback(timeline::Playback::Stop);
                _timeline->seek(value);
            }
        }

        void TimelineWidget::_timeSliderCallback(int value)
        {
            if (_timeline)
            {
                _timeline->setPlayback(timeline::Playback::Stop);
                _timeline->seek(otime::RationalTime(value, _timeline->getDuration().rate()));
            }
        }

        void TimelineWidget::_playbackUpdate()
        {
            timeline::Playback playback = timeline::Playback::Stop;
            if (_timeline)
            {
                playback = _timeline->getPlayback();
            }
            _playbackButtons[timeline::Playback::Stop]->setChecked(timeline::Playback::Stop == playback);
            _playbackButtons[timeline::Playback::Forward]->setChecked(timeline::Playback::Forward == playback);
        }

        void TimelineWidget::_timelineUpdate()
        {
            if (_timeline)
            {
                _playbackButtons[timeline::Playback::Stop]->setEnabled(true);
                _playbackButtons[timeline::Playback::Forward]->setEnabled(true);

                const otime::RationalTime& duration = _timeline->getDuration();
                _timeSlider->setRange(0, duration.value() > 0 ? duration.value() - 1 : 0);
                _timeSlider->setEnabled(true);

                otime::ErrorStatus errorStatus;
                std::string label = duration.to_timecode(&errorStatus);
                if (errorStatus != otime::ErrorStatus::OK)
                {
                    throw std::runtime_error(errorStatus.details);
                }
                _durationLabel->setText(label.c_str());

                connect(
                    _timeline,
                    SIGNAL(currentTimeChanged(const otime::RationalTime&)),
                    SLOT(_currentTimeCallback(const otime::RationalTime&)));
                connect(
                    _timeline,
                    SIGNAL(playbackChanged(tlr::timeline::Playback)),
                    SLOT(_playbackCallback(tlr::timeline::Playback)));
                connect(
                    _timeline,
                    SIGNAL(loopChanged(tlr::timeline::Loop)),
                    SLOT(_loopCallback(tlr::timeline::Loop)));
            }
            else
            {
                _playbackButtons[timeline::Playback::Stop]->setChecked(true);
                _playbackButtons[timeline::Playback::Stop]->setEnabled(false);
                _playbackButtons[timeline::Playback::Forward]->setChecked(false);
                _playbackButtons[timeline::Playback::Forward]->setEnabled(false);

                _currentTimeSpinBox->setValue(otime::RationalTime());

                _timeSlider->setRange(0, 0);
                _timeSlider->setValue(0);
                _timeSlider->setEnabled(false);

                _durationLabel->setText("00:00:00:00");
            }
        }
    }
}
