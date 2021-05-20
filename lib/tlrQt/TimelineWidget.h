// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021 Darby Johnston
// All rights reserved.

#pragma once

#include <tlrQt/SpeedLabel.h>
#include <tlrQt/TimeLabel.h>
#include <tlrQt/TimeSpinBox.h>
#include <tlrQt/TimelinePlayer.h>
#include <tlrQt/TimelineSlider.h>

#include <QButtonGroup>
#include <QMap>
#include <QPointer>
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

            //! Set the timeline player.
            void setTimelinePlayer(TimelinePlayer*);

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

            TimelinePlayer* _timelinePlayer = nullptr;
            QMap<QString, QAbstractButton*> _playbackButtons;
            QButtonGroup* _playbackButtonGroup = nullptr;
            QMap<QAbstractButton*, timeline::Playback> _buttonToPlayback;
            QMap<timeline::Playback, QAbstractButton*> _playbackToButton;
            QMap<QString, QAbstractButton*> _timeActionButtons;
            QButtonGroup* _timeActionButtonGroup = nullptr;
            QMap<QAbstractButton*, timeline::TimeAction> _buttonToTimeAction;
            SpeedLabel* _speedLabel = nullptr;
            TimelineSlider* _timelineSlider = nullptr;
            TimeSpinBox* _currentTimeSpinBox = nullptr;
            TimeSpinBox* _inPointSpinBox = nullptr;
            TimeSpinBox* _outPointSpinBox = nullptr;
            QMap<QString, QAbstractButton*> _inOutButtons;
            TimeLabel* _durationLabel = nullptr;
        };
    }
}
