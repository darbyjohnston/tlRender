// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021 Darby Johnston
// All rights reserved.

#pragma once

#include <tlrQt/TimelineObject.h>
#include <tlrQt/TimelineViewport.h>

#include <QAction>
#include <QLabel>
#include <QMap>
#include <QPointer>
#include <QSlider>

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

            //! Set the timeline.
            void setTimeline(qt::TimelineObject*);

        public Q_SLOTS:

        private Q_SLOTS:
            void _currentTimeCallback(const otime::RationalTime&);
            void _playbackCallback(tlr::timeline::Playback);
            void _loopCallback(tlr::timeline::Loop);
            void _stopCallback();
            void _forwardCallback();
            void _timeSliderCallback(int);

        private:
            void _playbackUpdate();
            void _timelineUpdate();

            QPointer<qt::TimelineObject> _timeline;
            QMap<std::string, QPointer<QAction> > _actions;
            QPointer<TimelineViewport> _viewport;
            QPointer<QLabel> _currentTimeLabel;
            QPointer<QSlider> _timeSlider;
            QPointer<QLabel> _durationLabel;
        };
    }
}
