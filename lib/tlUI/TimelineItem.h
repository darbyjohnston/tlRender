// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2023 Darby Johnston
// All rights reserved.

#pragma once

#include <tlUI/ITimelineItem.h>

#include <tlTimeline/TimelinePlayer.h>

namespace tl
{
    namespace ui
    {
        //! Timeline item.
        class TimelineItem : public ITimelineItem
        {
        protected:
            void _init(
                const std::shared_ptr<timeline::TimelinePlayer>&,
                const TimelineItemData&,
                const std::shared_ptr<system::Context>&,
                const std::shared_ptr<IWidget>& parent = nullptr);

            TimelineItem();

        public:
            ~TimelineItem() override;

            static std::shared_ptr<TimelineItem> create(
                const std::shared_ptr<timeline::TimelinePlayer>&,
                const TimelineItemData&,
                const std::shared_ptr<system::Context>&,
                const std::shared_ptr<IWidget>& parent = nullptr);

            void setStopOnScrub(bool);

            void setGeometry(const math::BBox2i&) override;
            void sizeHintEvent(const SizeHintEvent&) override;
            void drawEvent(const DrawEvent&) override;
            void enterEvent() override;
            void leaveEvent() override;
            void mouseMoveEvent(MouseMoveEvent&) override;
            void mousePressEvent(MouseClickEvent&) override;
            void mouseReleaseEvent(MouseClickEvent&) override;

        private:
            void _drawTimeTicks(const DrawEvent&);
            void _drawCurrentTime(const DrawEvent&);

            math::BBox2i _getCurrentTimeBBox() const;

            otime::RationalTime _posToTime(float) const;
            float _timeToPos(const otime::RationalTime&) const;

            TLRENDER_PRIVATE();
        };
    }
}
