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

            //! Create a new item.
            static std::shared_ptr<TimelineItem> create(
                const std::shared_ptr<timeline::TimelinePlayer>&,
                const TimelineItemData&,
                const std::shared_ptr<system::Context>&,
                const std::shared_ptr<IWidget>& parent = nullptr);

            //! Set whether playback stops when scrubbing.
            void setStopOnScrub(bool);

            void setGeometry(const math::BBox2i&) override;
            void setVisible(bool) override;
            void setEnabled(bool) override;
            void sizeHintEvent(const SizeHintEvent&) override;
            void clipEvent(
                const math::BBox2i&,
                bool,
                const ClipEvent&) override;
            void drawEvent(
                const math::BBox2i&,
                const DrawEvent&) override;
            void enterEvent() override;
            void leaveEvent() override;
            void mouseMoveEvent(MouseMoveEvent&) override;
            void mousePressEvent(MouseClickEvent&) override;
            void mouseReleaseEvent(MouseClickEvent&) override;

        private:
            void _drawTimeTicks(
                const math::BBox2i&,
                const DrawEvent&);
            void _drawCurrentTime(
                const math::BBox2i&,
                const DrawEvent&);

            otime::RationalTime _posToTime(float) const;
            float _timeToPos(const otime::RationalTime&) const;

            void _resetMouse();

            TLRENDER_PRIVATE();
        };
    }
}
