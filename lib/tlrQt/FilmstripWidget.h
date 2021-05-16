// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021 Darby Johnston
// All rights reserved.

#pragma once

#include <tlrQt/TimelineObject.h>

#include <tlrCore/Timeline.h>

#include <QMap>
#include <QPointer>
#include <QWidget>

#include <atomic>
#include <mutex>
#include <thread>

namespace tlr
{
    namespace qt
    {
        //! Filmstrip widget.
        class FilmstripWidget : public QWidget
        {
            Q_OBJECT

        public:
            FilmstripWidget(QWidget* parent = nullptr);
            ~FilmstripWidget() override;

        public Q_SLOTS:
            //! Set the timeline file name.
            void setFileName(const QString&);

        protected:
            void resizeEvent(QResizeEvent*) override;
            void paintEvent(QPaintEvent*) override;
            void timerEvent(QTimerEvent*) override;

        private Q_SLOTS:
            void _frameCallback(const tlr::io::VideoFrame&);

        private:
            otime::RationalTime _posToTime(int) const;
            int _timeToPos(const otime::RationalTime&) const;

            void _timelineUpdate();

            TimelineObject* _timeline = nullptr;
            QList<otime::RationalTime> _times;
            imaging::Size _thumbnailSize;
            QMap<otime::RationalTime, QImage> _thumbnails;

            std::condition_variable _thumbnailsCV;
            std::mutex _thumbnailMutex;
            std::list<std::pair<io::VideoFrame, imaging::Size> > _thumbnailRequests;
            std::list<std::pair<QImage, otime::RationalTime> > _thumbnailResults;
            std::thread _thumbnailThread;
            std::atomic<bool> _thumbnailThreadRunning;
        };
    }
}
