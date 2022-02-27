// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2022 Darby Johnston
// All rights reserved.

#pragma once

#include <tlQt/TimeObject.h>
#include <tlQt/TimelinePlayer.h>

#include <tlCore/OCIO.h>

#include <QWidget>

namespace tl
{
    namespace qt
    {
        namespace widget
        {
            //! Timeline slider.
            class TimelineSlider : public QWidget
            {
                Q_OBJECT
                Q_PROPERTY(
                    bool thumbnails
                    READ hasThumbnails
                    WRITE setThumbnails)
                Q_PROPERTY(
                    tl::qt::TimeUnits units
                    READ units
                    WRITE setUnits)

            public:
                TimelineSlider(
                    const std::shared_ptr<core::system::Context>&,
                    QWidget* parent = nullptr);

                ~TimelineSlider() override;

                //! Set the time object.
                void setTimeObject(qt::TimeObject*);

                //! Set the color configuration.
                void setColorConfig(const core::imaging::ColorConfig&);

                //! Set the timeline player.
                void setTimelinePlayer(qt::TimelinePlayer*);

                //! Get whether thumbnails are displayed.
                bool hasThumbnails() const;

                //! Get the time units.
                qt::TimeUnits units() const;

            public Q_SLOTS:
                //! Set whether thumbnails are displayed.
                void setThumbnails(bool);

                //! Set the time units.
                void setUnits(tl::qt::TimeUnits);

            protected:
                void resizeEvent(QResizeEvent*) override;
                void paintEvent(QPaintEvent*) override;
                void mousePressEvent(QMouseEvent*) override;
                void mouseReleaseEvent(QMouseEvent*) override;
                void mouseMoveEvent(QMouseEvent*) override;
                void wheelEvent(QWheelEvent*) override;

            private Q_SLOTS:
                void _thumbnailsCallback(const QList<QPair<otime::RationalTime, QImage> >&);

            private:
                otime::RationalTime _posToTime(int) const;
                int _timeToPos(const otime::RationalTime&) const;

                void _thumbnailsUpdate();

                TLRENDER_PRIVATE();
            };
        }
    }
}
