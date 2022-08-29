// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2022 Darby Johnston
// All rights reserved.

#pragma once

#include <tlQt/Util.h>

#include <tlTimeline/IRender.h>
#include <tlTimeline/Timeline.h>

#include <QImage>
#include <QThread>

namespace tl
{
    namespace qt
    {
        //! Timeline thumbnail provider.
        class TimelineThumbnailProvider : public QThread
        {
            Q_OBJECT

        public:
            TimelineThumbnailProvider(
                const std::shared_ptr<system::Context>&,
                QObject* parent = nullptr);
            ~TimelineThumbnailProvider() override;

            //! Request a thumbnail. The request ID is returned.
            qint64 request(
                const QString&,
                const otime::RationalTime&,
                const QSize&,
                const timeline::ColorConfigOptions& = timeline::ColorConfigOptions(),
                const timeline::LUTOptions& = timeline::LUTOptions());

            //! Request a thumbnail. The request ID is returned.
            qint64 request(
                const QString&,
                const QList<otime::RationalTime>&,
                const QSize&,
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
