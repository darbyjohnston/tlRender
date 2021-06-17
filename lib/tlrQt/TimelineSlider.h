// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021 Darby Johnston
// All rights reserved.

#pragma once

#include <tlrQt/TimeObject.h>
#include <tlrQt/TimelinePlayer.h>
#include <tlrQt/TimelineThumbnailProvider.h>

#include <tlrGL/Render.h>

#include <QWidget>

namespace tlr
{
    namespace qt
    {
        //! Timeline slider.
        class TimelineSlider : public QWidget
        {
            Q_OBJECT

        public:
            TimelineSlider(QWidget* parent = nullptr);

            //! Set the time object.
            void setTimeObject(TimeObject*);

            //! Set the color configuration.
            void setColorConfig(const gl::ColorConfig&);

            //! Set the timeline player.
            void setTimelinePlayer(TimelinePlayer*);

        public Q_SLOTS:
            //! Set the time units.
            void setUnits(qt::TimeObject::Units);

        protected:
            void resizeEvent(QResizeEvent*) override;
            void paintEvent(QPaintEvent*) override;
            void mousePressEvent(QMouseEvent*) override;
            void mouseReleaseEvent(QMouseEvent*) override;
            void mouseMoveEvent(QMouseEvent*) override;

        private Q_SLOTS:
            void _currentTimeCallback(const otime::RationalTime&);
            void _inOutRangeCallback(const otime::TimeRange&);
            void _cachedFramesCallback(const std::vector<otime::TimeRange>&);
            void _thumbnailsCallback(const QList<QPair<otime::RationalTime, QImage> >&);

        private:
            otime::RationalTime _posToTime(int) const;
            int _timeToPos(const otime::RationalTime&) const;

            void _thumbnailsUpdate();

            gl::ColorConfig _colorConfig;
            TimelinePlayer* _timelinePlayer = nullptr;
            TimelineThumbnailProvider* _thumbnailProvider = nullptr;
            std::map<otime::RationalTime, QImage> _thumbnails;
            TimeObject::Units _units = TimeObject::Units::Timecode;
            TimeObject* _timeObject = nullptr;
        };
    }
}
