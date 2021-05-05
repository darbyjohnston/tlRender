// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021 Darby Johnston
// All rights reserved.

#include <tlrQt/TimelineWidget.h>

#include <QFontDatabase>
#include <QHBoxLayout>
#include <QStyle>
#include <QToolBar>
#include <QVBoxLayout>

namespace tlr
{
    namespace qt
    {
        TimelineWidget::TimelineWidget(QWidget* parent) :
            QWidget(parent)
        {
            _actions["Stop"] = new QAction;
            _actions["Stop"]->setCheckable(true);
            _actions["Stop"]->setIcon(style()->standardIcon(QStyle::SP_MediaStop));
            _actions["Stop"]->setToolTip(tr("Stop playback"));
            _actions["Forward"] = new QAction;
            _actions["Forward"]->setCheckable(true);
            _actions["Forward"]->setIcon(style()->standardIcon(QStyle::SP_MediaPlay));
            _actions["Forward"]->setToolTip(tr("Forward playback"));

            _viewport = new TimelineViewport;

            const QFont fixedFont = QFontDatabase::systemFont(QFontDatabase::FixedFont);
            _currentTimeLabel = new QLabel;
            _currentTimeLabel->setMargin(10);
            _currentTimeLabel->setFont(fixedFont);
            _currentTimeLabel->setText("00:00:00:00");
            _currentTimeLabel->setToolTip(tr("Current time"));

            _timeSlider = new QSlider(Qt::Orientation::Horizontal);
            _timeSlider->setToolTip(tr("Time slider"));

            _durationLabel = new QLabel;
            _durationLabel->setMargin(10);
            _durationLabel->setFont(fixedFont);
            _durationLabel->setText("00:00:00:00");
            _durationLabel->setToolTip(tr("Duration"));

            auto playbackToolBar = new QToolBar;
            playbackToolBar->setMovable(false);
            playbackToolBar->setFloatable(false);
            playbackToolBar->addAction(_actions["Stop"]);
            playbackToolBar->addAction(_actions["Forward"]);
            playbackToolBar->addWidget(_currentTimeLabel);
            playbackToolBar->addWidget(_timeSlider);
            playbackToolBar->addWidget(_durationLabel);

            auto layout = new QVBoxLayout;
            layout->setMargin(0);
            layout->setSpacing(0);
            layout->addWidget(_viewport, 1);
            layout->addWidget(playbackToolBar);
            setLayout(layout);

            _playbackUpdate();
            _timelineUpdate();

            connect(
                _actions["Stop"],
                SIGNAL(triggered()),
                SLOT(_stopCallback()));
            connect(
                _actions["Forward"],
                SIGNAL(triggered()),
                SLOT(_forwardCallback()));
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
            _currentTimeLabel->setText(label.c_str());

            const QSignalBlocker blocker(_timeSlider);
            _timeSlider->setValue(value.value());
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
            }
        }

        void TimelineWidget::_forwardCallback()
        {
            if (_timeline)
            {
                _timeline->forward();
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
            _actions["Stop"]->setChecked(timeline::Playback::Stop == playback);
            _actions["Forward"]->setChecked(timeline::Playback::Forward == playback);
        }

        void TimelineWidget::_timelineUpdate()
        {
            if (_timeline)
            {
                _actions["Stop"]->setEnabled(true);
                _actions["Forward"]->setEnabled(true);

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
                _actions["Stop"]->setChecked(true);
                _actions["Stop"]->setEnabled(false);

                _currentTimeLabel->setText("00:00:00:00");

                _timeSlider->setRange(0, 0);
                _timeSlider->setValue(0);
                _timeSlider->setEnabled(false);

                _durationLabel->setText("00:00:00:00");
            }
        }
    }
}
