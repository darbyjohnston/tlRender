// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2025 Darby Johnston
// All rights reserved.

#pragma once

#include <tlTimelineUI/IItem.h>

#include <tlTimeline/Player.h>

namespace dtk
{
    namespace gl
    {
        class Window;
    }
}

namespace tl
{
    namespace timelineui
    {
        //! Track types.
        enum class TrackType
        {
            None,
            Video,
            Audio
        };

        //! Timeline item.
        //!
        //! \todo Add a selection model.
        //! \todo Add support for dragging clips to different tracks.
        //! \todo Add support for adjusting clip handles.
        //! \todo Add support for undo/redo.
        //! \todo Add an option for viewing/playing individual clips ("solo" mode).
        class TimelineItem : public IItem
        {
        protected:
            void _init(
                const std::shared_ptr<timeline::Player>&,
                const OTIO_NS::SerializableObject::Retainer<OTIO_NS::Stack>&,
                double scale,
                const ItemOptions&,
                const DisplayOptions&,
                const std::shared_ptr<ItemData>&,
                const std::shared_ptr<dtk::gl::Window>&,
                const std::shared_ptr<dtk::Context>&,
                const std::shared_ptr<IWidget>& parent);

            TimelineItem();

        public:
            virtual ~TimelineItem();

            //! Create a new item.
            static std::shared_ptr<TimelineItem> create(
                const std::shared_ptr<timeline::Player>&,
                const OTIO_NS::SerializableObject::Retainer<OTIO_NS::Stack>&,
                double scale,
                const ItemOptions&,
                const DisplayOptions&,
                const std::shared_ptr<ItemData>&,
                const std::shared_ptr<dtk::gl::Window>&,
                const std::shared_ptr<dtk::Context>&,
                const std::shared_ptr<IWidget>& parent = nullptr);

            //! Set whether the timeline is editable.
            void setEditable(bool);

            //! Set whether playback stops when scrubbing.
            void setStopOnScrub(bool);

            //! Observe whether scrubbing is in progress.
            std::shared_ptr<dtk::IObservableValue<bool> > observeScrub() const;

            //! Observe time scrubbing.
            std::shared_ptr<dtk::IObservableValue<OTIO_NS::RationalTime> > observeTimeScrub() const;

            //! Set the frame markers.
            void setFrameMarkers(const std::vector<int>&);

            //! Get the minimum height.
            int getMinimumHeight() const;

            //! Get the track geometry.
            std::vector<dtk::Box2I> getTrackGeom() const;

            void setDisplayOptions(const DisplayOptions&) override;

            void setGeometry(const dtk::Box2I&) override;
            void sizeHintEvent(const ui::SizeHintEvent&) override;
            void drawOverlayEvent(const dtk::Box2I&, const ui::DrawEvent&) override;
            void mouseMoveEvent(ui::MouseMoveEvent&) override;
            void mousePressEvent(ui::MouseClickEvent&) override;
            void mouseReleaseEvent(ui::MouseClickEvent&) override;
            //void keyPressEvent(ui::KeyEvent&) override;
            //void keyReleaseEvent(ui::KeyEvent&) override;

        protected:
            void _timeUnitsUpdate() override;

            void _releaseMouse() override;

        private:
            bool _isTrackVisible(int) const;
            void _setTrackEnabled(int, bool);

            void _drawInOutPoints(
                const dtk::Box2I&,
                const ui::DrawEvent&);
            dtk::Size2I _getLabelMaxSize(
                const std::shared_ptr<dtk::FontSystem>&) const;
            void _getTimeTicks(
                const std::shared_ptr<dtk::FontSystem>&,
                double& seconds,
                int& tick);
            void _drawTimeTicks(
                const dtk::Box2I&,
                const ui::DrawEvent&);
            void _drawFrameMarkers(
                const dtk::Box2I&,
                const ui::DrawEvent&);
            void _drawTimeLabels(
                const dtk::Box2I&,
                const ui::DrawEvent&);
            void _drawCacheInfo(
                const dtk::Box2I&,
                const ui::DrawEvent&);
            void _drawCurrentTime(
                const dtk::Box2I&,
                const ui::DrawEvent&);

            void _tracksUpdate();
            void _textUpdate();

            TLRENDER_PRIVATE();
        };
    }
}
