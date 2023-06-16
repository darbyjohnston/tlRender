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
        //! 
        //! \tool Keep the current frame display inside widget geometry.
        class TimelineWidget : public ui::IWidget
        {
            TLRENDER_NON_COPYABLE(TimelineWidget);

        protected:
            void _init(
                const std::shared_ptr<timeline::TimeUnitsModel>&,
                const std::shared_ptr<system::Context>&,
                const std::shared_ptr<IWidget>& parent = nullptr);

            TimelineWidget();

        public:
            ~TimelineWidget() override;

            //! Create a new widget.
            static std::shared_ptr<TimelineWidget> create(
                const std::shared_ptr<timeline::TimeUnitsModel>&,
                const std::shared_ptr<system::Context>&,
                const std::shared_ptr<IWidget>& parent = nullptr);

            //! Set the timeline player.
            void setPlayer(const std::shared_ptr<timeline::Player>&);

            //! Set the view zoom.
            void setViewZoom(double);

            //! Set the view zoom.
            void setViewZoom(
                double,
                const tl::math::Vector2i& focus);

            //! Frame the view.
            void frameView();

            //! Get whether the view is framed automatically.
            bool hasFrameView() const;

            //! Set whether the view is framed automatically.
            void setFrameView(bool);

            //! Set the frame view callback.
            void setFrameViewCallback(const std::function<void(bool)>&);

            //! Get whether the scroll bars are visible.
            bool areScrollBarsVisible() const;

            //! Set whether the scroll bars are visible.
            void setScrollBarsVisible(bool);

            //! Get the mouse scroll key modifier.
            ui::KeyModifier getScrollKeyModifier() const;

            //! Set the mouse scroll key modifier.
            void setScrollKeyModifier(ui::KeyModifier);

            //! Get whether to stop playback when scrubbing.
            bool hasStopOnScrub() const;

            //! Set whether to stop playback when scrubbing.
            void setStopOnScrub(bool);

            //! Get the mouse wheel scale.
            float getMouseWheelScale() const;

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
                double zoomNew,
                double zoomPrev,
                const math::Vector2i& focus,
                const math::Vector2i& scrollPos);

            double _getTimelineScale() const;

            void _setItemScale(
                const std::shared_ptr<IWidget>&,
                double);
            void _setItemOptions(
                const std::shared_ptr<IWidget>&,
                const ItemOptions&);

            void _resetMouse();

            TLRENDER_PRIVATE();
        };
    }
}
