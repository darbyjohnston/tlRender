// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021 Darby Johnston
// All rights reserved.

#pragma once

#include <tlrQt/TimeLabel.h>
#include <tlrQt/TimeSpinBox.h>
#include <tlrQt/TimelineObject.h>

#include <QMap>
#include <QPointer>
#include <QSlider>
#include <QToolButton>

namespace tlr
{
    namespace qt
    {
        //! Timeline widget.
        class TimelineWidget : public QWidget
        {
            Q_OBJECT

        public:
            TimelineWidget(QWidget* parent = nullptr);

            //! Set the time object.
            void setTimeObject(TimeObject*);

            //! Set the timeline.
            void setTimeline(TimelineObject*);

        public Q_SLOTS:

        private Q_SLOTS:
            void _currentTimeCallback(const otime::RationalTime&);
            void _playbackCallback(tlr::timeline::Playback);
            void _loopCallback(tlr::timeline::Loop);
            void _stopCallback();
            void _forwardCallback();
            void _currentTimeSpinBoxCallback(const otime::RationalTime&);
            void _timeSliderCallback(int);

        private:
            void _playbackUpdate();
            void _timelineUpdate();

            QPointer<TimelineObject> _timeline;
            QMap<timeline::Playback, QPointer<QToolButton> > _playbackButtons;
            QPointer<TimeSpinBox> _currentTimeSpinBox;
            QPointer<QSlider> _timeSlider;
            QPointer<TimeLabel> _durationLabel;
        };
    }
}
