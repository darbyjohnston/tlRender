// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021 Darby Johnston
// All rights reserved.

#pragma once

#include <tlrQt/TimeObject.h>
#include <tlrQt/TimelinePlayer.h>

#include <QAbstractButton>

namespace tlr
{
    namespace qwidget
    {
        //! Timeline controls.
        class TimelineControls : public QWidget
        {
            Q_OBJECT

        public:
            TimelineControls(QWidget* parent = nullptr);
            
            ~TimelineControls() override;

            //! Set the time object.
            void setTimeObject(qt::TimeObject*);

            //! Set the timeline player.
            void setTimelinePlayer(qt::TimelinePlayer*);

        private Q_SLOTS:
            void _playbackCallback(QAbstractButton*);
            void _playbackCallback(tlr::timeline::Playback);
            void _timeActionCallback(QAbstractButton*);
            void _currentTimeCallback(const otime::RationalTime&);
            void _currentTimeCallback2(const otime::RationalTime&);
            void _inPointCallback(const otime::RationalTime&);
            void _inPointCallback();
            void _resetInPointCallback();
            void _outPointCallback(const otime::RationalTime&);
            void _outPointCallback();
            void _resetOutPointCallback();
            void _inOutRangeCallback(const otime::TimeRange&);

        private:
            void _playbackUpdate();
            void _timelineUpdate();

            TLR_PRIVATE();
        };
    }
}
