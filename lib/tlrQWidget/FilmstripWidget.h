// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2022 Darby Johnston
// All rights reserved.

#pragma once

#include <tlrQt/Util.h>

#include <tlrCore/Timeline.h>

#include <QWidget>

namespace tlr
{
    namespace qwidget
    {
        //! Filmstrip widget.
        class FilmstripWidget : public QWidget
        {
            Q_OBJECT

        public:
            FilmstripWidget(QWidget* parent = nullptr);
            
            ~FilmstripWidget() override;

            //! Set the timeline.
            void setTimeline(const std::shared_ptr<tlr::timeline::Timeline>&);

        public Q_SLOTS:
            //! Set the row count.
            void setRowCount(int);

        protected:
            void resizeEvent(QResizeEvent*) override;
            void paintEvent(QPaintEvent*) override;

        private Q_SLOTS:
            void _thumbnailsCallback(const QList<QPair<otime::RationalTime, QImage> >&);

        private:
            void _thumbnailsUpdate();

            TLR_PRIVATE();
        };
    }
}
