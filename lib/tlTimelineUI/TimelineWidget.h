// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2025 Darby Johnston
// All rights reserved.

#pragma once

#include <tlTimelineUI/TimelineItem.h>

namespace tl
{
    namespace timelineui
    {
        //! Timeline widget.
        //! 
        //! \tool Adjust the current frame label to stay visible on the right
        //! side of the timeline widget.
        class TimelineWidget : public dtk::IWidget
        {
            DTK_NON_COPYABLE(TimelineWidget);

        protected:
            void _init(
                const std::shared_ptr<dtk::Context>&,
                const std::shared_ptr<timeline::ITimeUnitsModel>&,
                const std::shared_ptr<IWidget>& parent);

            TimelineWidget();

        public:
            virtual ~TimelineWidget();

            //! Create a new widget.
            static std::shared_ptr<TimelineWidget> create(
                const std::shared_ptr<dtk::Context>&,
                const std::shared_ptr<timeline::ITimeUnitsModel>&,
                const std::shared_ptr<IWidget>& parent = nullptr);

            //! Get the timeline player.
            std::shared_ptr<timeline::Player>& getPlayer() const;

            //! Set the timeline player.
            void setPlayer(const std::shared_ptr<timeline::Player>&);

            //! \name Editing
            ///@{

            //! Get whether the timeline is editable.
            bool isEditable() const;

            //! Observe whether the timeline is editable.
            std::shared_ptr<dtk::IObservableValue<bool> > observeEditable() const;

            //! Set whether the timeline is editable.
            void setEditable(bool);

            ///@}

            //! \name View
            ///@{

            //! Set the view zoom.
            void setViewZoom(double);

            //! Set the view zoom.
            void setViewZoom(
                double,
                const dtk::V2I& focus);

            //! Frame the view.
            void frameView();

            //! Get whether the view is framed automatically.
            bool hasFrameView() const;
            
            //! Observe whether the view is framed automatically.
            std::shared_ptr<dtk::IObservableValue<bool> > observeFrameView() const;

            //! Set whether the view is framed automatically.
            void setFrameView(bool);

            //! Get whether the scroll bars are visible.
            bool areScrollBarsVisible() const;

            //! Set whether the scroll bars are visible.
            void setScrollBarsVisible(bool);

            //! Get whether to automatically scroll to the current frame.
            bool hasScrollToCurrentFrame() const;

            //! Observe whether to automatically scroll to the current frame.
            std::shared_ptr<dtk::IObservableValue<bool> > observeScrollToCurrentFrame() const;

            //! Set whether to automatically scroll to the current frame.
            void setScrollToCurrentFrame(bool);

            //! Get the mouse scroll key modifier.
            dtk::KeyModifier getScrollKeyModifier() const;

            //! Set the mouse scroll key modifier.
            void setScrollKeyModifier(dtk::KeyModifier);

            //! Get the mouse wheel scale.
            float getMouseWheelScale() const;

            //! Set the mouse wheel scale.
            void setMouseWheelScale(float);

            ///@}

            //! \name Scrubbing
            ///@{

            //! Get whether to stop playback when scrubbing.
            bool hasStopOnScrub() const;

            //! Observe whether to stop playback when scrubbing.
            std::shared_ptr<dtk::IObservableValue<bool> > observeStopOnScrub() const;

            //! Set whether to stop playback when scrubbing.
            void setStopOnScrub(bool);

            //! Observe whether scrubbing is in progress.
            std::shared_ptr<dtk::IObservableValue<bool> > observeScrub() const;

            //! Observe time scrubbing.
            std::shared_ptr<dtk::IObservableValue<OTIO_NS::RationalTime> > observeTimeScrub() const;

            ///@}

            //! \name Frame Markers
            ///@{

            //! Get the frame markers.
            const std::vector<int>& getFrameMarkers() const;

            //! Set the frame markers.
            void setFrameMarkers(const std::vector<int>&);

            ///@}

            //! \name Options
            ///@{

            //! Get the item options.
            const ItemOptions& getItemOptions() const;

            //! Observe the item options.
            std::shared_ptr<dtk::IObservableValue<ItemOptions> > observeItemOptions() const;

            //! Set the item options.
            void setItemOptions(const ItemOptions&);

            //! Get the display options.
            const DisplayOptions& getDisplayOptions() const;

            //! Observe the display options.
            std::shared_ptr<dtk::IObservableValue<DisplayOptions> > observeDisplayOptions() const;

            //! Set the display options.
            void setDisplayOptions(const DisplayOptions&);

            ///@}

            //! Get the track geometry.
            std::vector<dtk::Box2I> getTrackGeom() const;

            void setGeometry(const dtk::Box2I&) override;
            void tickEvent(
                bool,
                bool,
                const dtk::TickEvent&) override;
            void sizeHintEvent(const dtk::SizeHintEvent&) override;
            void mouseMoveEvent(dtk::MouseMoveEvent&) override;
            void mousePressEvent(dtk::MouseClickEvent&) override;
            void mouseReleaseEvent(dtk::MouseClickEvent&) override;
            void scrollEvent(dtk::ScrollEvent&) override;
            void keyPressEvent(dtk::KeyEvent&) override;
            void keyReleaseEvent(dtk::KeyEvent&) override;

        protected:
            void _releaseMouse() override;

        private:
            void _setViewZoom(
                double zoomNew,
                double zoomPrev,
                const dtk::V2I& focus,
                const dtk::V2I& scrollPos);

            double _getTimelineScale() const;
            double _getTimelineScaleMax() const;

            void _setItemScale();
            void _setItemScale(
                const std::shared_ptr<IWidget>&,
                double);
            void _setItemOptions(
                const std::shared_ptr<IWidget>&,
                const ItemOptions&);
            void _setDisplayOptions(
                const std::shared_ptr<IWidget>&,
                const DisplayOptions&);

            void _scrollUpdate();
            void _timelineUpdate();

            DTK_PRIVATE();
        };
    }
}
