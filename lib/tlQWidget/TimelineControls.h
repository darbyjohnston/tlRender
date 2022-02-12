// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2022 Darby Johnston
// All rights reserved.

#pragma once

#include <tlQt/TimeObject.h>
#include <tlQt/TimelinePlayer.h>

#include <QAbstractButton>

namespace tl
{
    namespace qwidget
    {
        //! Timeline controls.
        class TimelineControls : public QWidget
        {
            Q_OBJECT
            Q_PROPERTY(
                QList<double> speeds
                READ speeds
                WRITE setSpeeds
                NOTIFY speedsChanged)

        public:
            TimelineControls(QWidget* parent = nullptr);
            
            ~TimelineControls() override;

            //! Set the time object.
            void setTimeObject(qt::TimeObject*);

            //! Set the timeline player.
            void setTimelinePlayer(qt::TimelinePlayer*);

            //! Get the list of speeds.
            const QList<double>& speeds() const;

        public Q_SLOTS:
            //! Set the list of speeds.
            void setSpeeds(const QList<double>&);

            //! Focus the current frame widget.
            void focusCurrentFrame();

        Q_SIGNALS:
            //! This signal is emitted when the list of speeds is changed.
            void speedsChanged(const QList<double>&);

        private Q_SLOTS:
            void _speedCallback(double);
            void _speedCallback2(double);
            void _speedCallback(QAction*);
            void _playbackCallback(QAbstractButton*);
            void _playbackCallback(tl::timeline::Playback);
            void _timeActionCallback(QAbstractButton*);
            void _currentTimeCallback(const otime::RationalTime&);
            void _currentTimeCallback2(const otime::RationalTime&);
            void _volumeCallback(int);
            void _volumeCallback2(float);
            void _muteCallback(bool);
            void _muteCallback2(bool);

        private:
            void _playbackUpdate();
            void _widgetUpdate();

            TLRENDER_PRIVATE();
        };
    }
}
