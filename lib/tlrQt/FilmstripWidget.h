// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021 Darby Johnston
// All rights reserved.

#pragma once

#include <tlrCore/Timeline.h>

#include <QWidget>

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
            //! Set the timeline.
            void setTimeline(const std::shared_ptr<tlr::timeline::Timeline>&);

        protected:
            void resizeEvent(QResizeEvent*) override;
            void paintEvent(QPaintEvent*) override;

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
