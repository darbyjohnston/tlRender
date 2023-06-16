// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2023 Darby Johnston
// All rights reserved.

#pragma once

#include <tlTimeline/IRender.h>
#include <tlTimeline/Timeline.h>

#include <QImage>
#include <QThread>

namespace tl
{
    namespace qt
    {
        //! Timeline thumbnail object.
        class TimelineThumbnailObject : public QThread
        {
            Q_OBJECT

        public:
            TimelineThumbnailObject(
                const std::shared_ptr<system::Context>&,
                QObject* parent = nullptr);
            ~TimelineThumbnailObject() override;

            //! Request a thumbnail. The request ID is returned.
            qint64 request(
                const QString&,
                const QSize&,
                const otime::RationalTime& = time::invalidTime,
                const timeline::ColorConfigOptions& = timeline::ColorConfigOptions(),
                const timeline::LUTOptions& = timeline::LUTOptions());

            //! Request multiple thumbnails. The request ID is returned.
            qint64 request(
                const QString&,
                const QSize&,
                const QList<otime::RationalTime>&,
                const timeline::ColorConfigOptions & = timeline::ColorConfigOptions(),
                const timeline::LUTOptions& = timeline::LUTOptions());

            //! Cancel thumbnail requests.
            void cancelRequests(qint64);

            //! Set the request count.
            void setRequestCount(int);

            //! Set the request timeout (milliseconds).
            void setRequestTimeout(int);

            //! Set the timer interval (milliseconds).
            void setTimerInterval(int);

        Q_SIGNALS:
            //! This signal is emitted when thumbnails are ready.
            void thumbails(qint64, const QList<QPair<otime::RationalTime, QImage> >&);

        protected:
            void run() override;
            void timerEvent(QTimerEvent*) override;

        private:
            TLRENDER_PRIVATE();
        };
    }
}
