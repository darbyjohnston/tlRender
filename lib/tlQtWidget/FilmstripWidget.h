// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2023 Darby Johnston
// All rights reserved.

#pragma once

#include <tlTimeline/Timeline.h>

#include <QWidget>

namespace tl
{
    namespace qt
    {
        class TimelineThumbnailObject;
    }

    namespace qtwidget
    {
        //! Filmstrip widget.
        class FilmstripWidget : public QWidget
        {
            Q_OBJECT
            Q_PROPERTY(
                int rowCount
                READ rowCount
                WRITE setRowCount)

        public:
            FilmstripWidget(qt::TimelineThumbnailObject*, QWidget* parent = nullptr);

            ~FilmstripWidget() override;

            //! Set the timeline.
            void setTimeline(const std::shared_ptr<timeline::Timeline>&);

            //! Get the row count.
            int rowCount() const;

        public Q_SLOTS:
            //! Set the row count.
            void setRowCount(int);

        protected:
            void resizeEvent(QResizeEvent*) override;
            void paintEvent(QPaintEvent*) override;

        private Q_SLOTS:
            void _thumbnailsCallback(qint64, const QList<QPair<otime::RationalTime, QImage> >&);

        private:
            void _thumbnailsUpdate();

            TLRENDER_PRIVATE();
        };
    }
}
