// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2022 Darby Johnston
// All rights reserved.

#pragma once

#include <tlrQt/TimeObject.h>
#include <tlrQt/TimelinePlayer.h>

#include <tlrCore/OCIO.h>

#include <QWidget>

namespace tlr
{
    namespace qwidget
    {
        //! Timeline slider.
        class TimelineSlider : public QWidget
        {
            Q_OBJECT

        public:
            TimelineSlider(QWidget* parent = nullptr);

            ~TimelineSlider() override;
            
            //! Set the time object.
            void setTimeObject(qt::TimeObject*);

            //! Set the color configuration.
            void setColorConfig(const imaging::ColorConfig&);

            //! Set the timeline player.
            void setTimelinePlayer(qt::TimelinePlayer*);

        public Q_SLOTS:
            //! Set the time units.
            void setUnits(tlr::qt::TimeUnits);

        protected:
            void resizeEvent(QResizeEvent*) override;
            void paintEvent(QPaintEvent*) override;
            void mousePressEvent(QMouseEvent*) override;
            void mouseReleaseEvent(QMouseEvent*) override;
            void mouseMoveEvent(QMouseEvent*) override;

        private Q_SLOTS:
            void _thumbnailsCallback(const QList<QPair<otime::RationalTime, QImage> >&);

        private:
            otime::RationalTime _posToTime(int) const;
            int _timeToPos(const otime::RationalTime&) const;

            void _thumbnailsUpdate();

            TLR_PRIVATE();
        };
    }
}
