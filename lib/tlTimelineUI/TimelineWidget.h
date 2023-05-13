// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2023 Darby Johnston
// All rights reserved.

#pragma once

#include <tlTimelineUI/TimelineItem.h>

namespace tl
{
    namespace timelineui
    {
        //! Timeline widget.
        class TimelineWidget : public ui::IWidget
        {
            TLRENDER_NON_COPYABLE(TimelineWidget);

        protected:
            void _init(
                const std::shared_ptr<system::Context>&,
                const std::shared_ptr<IWidget>& parent = nullptr);

            TimelineWidget();

        public:
            ~TimelineWidget() override;

            //! Create a new widget.
            static std::shared_ptr<TimelineWidget> create(
                const std::shared_ptr<system::Context>&,
                const std::shared_ptr<IWidget>& parent = nullptr);

            //! Set the timeline player.
            void setPlayer(const std::shared_ptr<timeline::Player>&);

            //! Set the view zoom.
            void setViewZoom(float);

            //! Set the view zoom.
            void setViewZoom(
                float,
                const tl::math::Vector2i& focus);

            //! Frame the view.
            void frameView();

            //! Set whether the view is framed automatically.
            void setFrameView(bool);

            //! Set the frame view callback.
            void setFrameViewCallback(const std::function<void(bool)>&);

            //! Set whether to stop playback when scrubbing.
            void setStopOnScrub(bool);

            //! Set the mouse wheel scale.
            void setMouseWheelScale(float);

            //! Get the item options.
            const ItemOptions& getItemOptions() const;

            //! Set the item options.
            void setItemOptions(const ItemOptions&);

            void setGeometry(const math::BBox2i&) override;
            void setVisible(bool) override;
            void setEnabled(bool) override;
            void sizeHintEvent(const ui::SizeHintEvent&) override;
            void clipEvent(
                const math::BBox2i&,
                bool,
                const ui::ClipEvent&) override;
            void mouseMoveEvent(ui::MouseMoveEvent&) override;
            void mousePressEvent(ui::MouseClickEvent&) override;
            void mouseReleaseEvent(ui::MouseClickEvent&) override;
            void scrollEvent(ui::ScrollEvent&) override;
            void keyPressEvent(ui::KeyEvent&) override;
            void keyReleaseEvent(ui::KeyEvent&) override;

        private:
            void _setViewZoom(
                float zoomNew,
                float zoomPrev,
                const math::Vector2i& focus,
                const math::Vector2i& scrollPos);

            float _getTimelineScale() const;

            void _setItemOptions(
                const std::shared_ptr<IWidget>&,
                const ItemOptions&);

            void _resetMouse();

            TLRENDER_PRIVATE();
        };
    }
}
