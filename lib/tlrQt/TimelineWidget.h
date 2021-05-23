// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021 Darby Johnston
// All rights reserved.

#pragma once

#include <tlrQt/TimelineControls.h>
#include <tlrQt/TimelinePlayer.h>
#include <tlrQt/TimelineSlider.h>
#include <tlrQt/TimelineViewport.h>

namespace tlr
{
    namespace qt
    {
        //! Timeline widget.
        class TimelineWidget : public QWidget
        {
            Q_OBJECT

        public:
            TimelineWidget(QWidget* parent = nullptr);

            //! Set the time object.
            void setTimeObject(TimeObject*);

            //! Set the timeline player.
            void setTimelinePlayer(TimelinePlayer*);

        private:
            TimelineViewport* _viewport = nullptr;
            TimelineSlider* _slider = nullptr;
            TimelineControls* _controls = nullptr;
        };
    }
}
