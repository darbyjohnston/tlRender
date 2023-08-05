// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2023 Darby Johnston
// All rights reserved.

#pragma once

#include <tlTimelineUI/IItem.h>

#include <tlTimeline/Player.h>

namespace tl
{
    namespace timelineui
    {
        //! Timeline item.
        class TimelineItem : public IItem
        {
        protected:
            void _init(
                const std::shared_ptr<timeline::Player>&,
                const ItemData&,
                const std::shared_ptr<system::Context>&,
                const std::shared_ptr<IWidget>& parent);

            TimelineItem();

        public:
            virtual ~TimelineItem();

            //! Create a new item.
            static std::shared_ptr<TimelineItem> create(
                const std::shared_ptr<timeline::Player>&,
                const ItemData&,
                const std::shared_ptr<system::Context>&,
                const std::shared_ptr<IWidget>& parent = nullptr);

            //! Set whether playback stops when scrubbing.
            void setStopOnScrub(bool);

            void setVisible(bool) override;
            void setEnabled(bool) override;
            void setGeometry(const math::Box2i&) override;
            void sizeHintEvent(const ui::SizeHintEvent&) override;
            void clipEvent(
                const math::Box2i&,
                bool,
                const ui::ClipEvent&) override;
            void drawOverlayEvent(
                const math::Box2i&,
                const ui::DrawEvent&) override;
            void mouseEnterEvent() override;
            void mouseLeaveEvent() override;
            void mouseMoveEvent(ui::MouseMoveEvent&) override;
            void mousePressEvent(ui::MouseClickEvent&) override;
            void mouseReleaseEvent(ui::MouseClickEvent&) override;
            //void keyPressEvent(ui::KeyEvent&) override;
            //void keyReleaseEvent(ui::KeyEvent&) override;

        protected:
            void _timeUnitsUpdate() override;

        private:
            void _drawInOutPoints(
                const math::Box2i&,
                const ui::DrawEvent&);
            void _drawTimeTicks(
                const math::Box2i&,
                const ui::DrawEvent&);
            void _drawCacheInfo(
                const math::Box2i&,
                const ui::DrawEvent&);
            void _drawCurrentTime(
                const math::Box2i&,
                const ui::DrawEvent&);

            otime::RationalTime _posToTime(float) const;
            int _timeToPos(const otime::RationalTime&) const;

            void _resetMouse();

            TLRENDER_PRIVATE();
        };
    }
}
