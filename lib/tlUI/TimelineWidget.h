// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2023 Darby Johnston
// All rights reserved.

#pragma once

#include <tlUI/TimelineItem.h>

namespace tl
{
    namespace ui
    {
        //! Timeline widget.
        class TimelineWidget : public IWidget
        {
            TLRENDER_NON_COPYABLE(TimelineWidget);

        protected:
            void _init(
                const std::shared_ptr<system::Context>&,
                const std::shared_ptr<IWidget>& parent = nullptr);

            TimelineWidget();

        public:
            ~TimelineWidget() override;

            //! Create a new timeline widget.
            static std::shared_ptr<TimelineWidget> create(
                const std::shared_ptr<system::Context>&,
                const std::shared_ptr<IWidget>& parent = nullptr);

            //! Set the timeline player.
            void setTimelinePlayer(const std::shared_ptr<timeline::TimelinePlayer>&);

            //! Set the view zoom.
            void setViewZoom(float);

            //! Set the view zoom.
            void setViewZoom(
                float,
                const tl::math::Vector2i& focus);

            //! Set whether the view is framed.
            void setFrameView(bool);

            //! Set the frame view callback.
            void setFrameViewCallback(const std::function<void(bool)>&);

            //! Set whether to stop playback when scrubbing.
            void setStopOnScrub(bool);

            //! Set the mouse wheel scale.
            void setMouseWheelScale(float);

            //! Get the item options.
            const TimelineItemOptions& itemOptions() const;

            //! Set the item options.
            void setItemOptions(const TimelineItemOptions&);

            void setGeometry(const math::BBox2i&) override;
            void setVisible(bool) override;
            void setEnabled(bool) override;
            void sizeHintEvent(const SizeHintEvent&) override;
            void clipEvent(bool, const ClipEvent&) override;
            void mouseMoveEvent(MouseMoveEvent&) override;
            void mousePressEvent(MouseClickEvent&) override;
            void mouseReleaseEvent(MouseClickEvent&) override;
            void keyPressEvent(KeyEvent&) override;
            void keyReleaseEvent(KeyEvent&) override;

        private:
            void _setScrollPos(const math::Vector2i&);

            void _frameView();

            void _setViewZoom(
                float zoomNew,
                float zoomPrev,
                const math::Vector2i& focus,
                const math::Vector2i& scrollPos);

            float _getTimelineScale() const;
            void _setItemOptions(
                const std::shared_ptr<IWidget>&,
                const TimelineItemOptions&);

            math::BBox2i _getTimelineViewport() const;
            void _setViewport(
                const std::shared_ptr<IWidget>&,
                const math::BBox2i&);

            void _resetMouse();

            TLRENDER_PRIVATE();
        };
    }
}
