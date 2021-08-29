// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021 Darby Johnston
// All rights reserved.

#pragma once

#include <tlrQt/Util.h>

#include <tlrGL/Render.h>

#include <tlrCore/Timeline.h>

#include <QImage>
#include <QThread>

namespace tlr
{
    namespace qt
    {
        //! Timeout for thumbnail requests.
        const std::chrono::milliseconds thumbnailRequestTimeout(1);

        //! The thumbnail timer interval.
        const int thumbnailTimerInterval = 10;

        //! Timeline thumbnail provider.
        class TimelineThumbnailProvider : public QThread
        {
            Q_OBJECT

        public:
            TimelineThumbnailProvider(
                const std::shared_ptr<tlr::timeline::Timeline>&,
                QObject* parent = nullptr);
            ~TimelineThumbnailProvider() override;

            //! Set the color configuration.
            void setColorConfig(const gl::ColorConfig&);

        public Q_SLOTS:
            //! Request a thumbnail.
            void request(const otime::RationalTime&, const QSize&);

            //! Request thumbnails.
            void request(const QList<otime::RationalTime>&, const QSize&);

            //! Cancel all thumbnail requests.
            void cancelRequests();

        Q_SIGNALS:
            //! This signal is emitted when thumbnails are ready.
            void thumbails(const QList<QPair<otime::RationalTime, QImage> >&);

        protected:
            void run() override;
            void timerEvent(QTimerEvent*) override;

        private:
            TLR_PRIVATE();
        };
    }
}
