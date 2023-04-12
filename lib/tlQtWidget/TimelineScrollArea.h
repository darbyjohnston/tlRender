// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2023 Darby Johnston
// All rights reserved.

#pragma once

#include <tlQtWidget/TimelineWidget.h>

#include <QAbstractScrollArea>

namespace tl
{
    namespace qtwidget
    {
        //! Timeline scroll area.
        class TimelineScrollArea : public QAbstractScrollArea
        {
            Q_OBJECT

        public:
            TimelineScrollArea(QWidget* parent = nullptr);

            ~TimelineScrollArea() override;

            void setTimelineWidget(TimelineWidget*);

        protected:
            void resizeEvent(QResizeEvent*);

        private:
            void _sizeUpdate();

            TLRENDER_PRIVATE();
        };
    }
}
