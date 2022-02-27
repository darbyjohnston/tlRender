// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2022 Darby Johnston
// All rights reserved.

#pragma once

#include <tlQt/Util.h>

#include <tlTimeline/Timeline.h>

#include <tlCore/OCIO.h>

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
                const std::shared_ptr<timeline::Timeline>&,
                const std::shared_ptr<system::Context>&,
                QObject* parent = nullptr);
            ~TimelineThumbnailProvider() override;

            //! Set the color configuration.
            void setColorConfig(const imaging::ColorConfig&);

        public Q_SLOTS:
            //! Request a thumbnail.
            void request(const otime::RationalTime&, const QSize&);

            //! Request thumbnails.
            void request(const QList<otime::RationalTime>&, const QSize&);

            //! Cancel all thumbnail requests.
            void cancelRequests();

            //! Set the request count.
            void setRequestCount(int);

            //! Set the request timeout (milliseconds).
            void setRequestTimeout(int);

            //! Set the timer interval (milliseconds).
            void setTimerInterval(int);

        Q_SIGNALS:
            //! This signal is emitted when thumbnails are ready.
            void thumbails(const QList<QPair<otime::RationalTime, QImage> >&);

        protected:
            void run() override;
            void timerEvent(QTimerEvent*) override;

        private:
            TLRENDER_PRIVATE();
        };
    }
}
