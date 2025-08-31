// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2025 Darby Johnston
// All rights reserved.

#pragma once

#include <tlTimelineUI/IItem.h>

#include <tlTimeline/Player.h>

namespace ftk
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
                const std::shared_ptr<ftk::Context>&,
                const std::shared_ptr<timeline::Player>&,
                const OTIO_NS::SerializableObject::Retainer<OTIO_NS::Stack>&,
                double scale,
                const ItemOptions&,
                const DisplayOptions&,
                const std::shared_ptr<ItemData>&,
                const std::shared_ptr<ftk::gl::Window>&,
                const std::shared_ptr<IWidget>& parent);

            TimelineItem();

        public:
            virtual ~TimelineItem();

            //! Create a new item.
            static std::shared_ptr<TimelineItem> create(
                const std::shared_ptr<ftk::Context>&,
                const std::shared_ptr<timeline::Player>&,
                const OTIO_NS::SerializableObject::Retainer<OTIO_NS::Stack>&,
                double scale,
                const ItemOptions&,
                const DisplayOptions&,
                const std::shared_ptr<ItemData>&,
                const std::shared_ptr<ftk::gl::Window>&,
                const std::shared_ptr<IWidget>& parent = nullptr);

            //! Set whether playback stops when scrubbing.
            void setStopOnScrub(bool);

            //! Observe whether scrubbing is in progress.
            std::shared_ptr<ftk::IObservableValue<bool> > observeScrub() const;

            //! Observe time scrubbing.
            std::shared_ptr<ftk::IObservableValue<OTIO_NS::RationalTime> > observeTimeScrub() const;

            //! Set the frame markers.
            void setFrameMarkers(const std::vector<int>&);

            //! Get the track geometry.
            std::vector<ftk::Box2I> getTrackGeom() const;

            void setDisplayOptions(const DisplayOptions&) override;

            void setGeometry(const ftk::Box2I&) override;
            void sizeHintEvent(const ftk::SizeHintEvent&) override;
            void drawOverlayEvent(const ftk::Box2I&, const ftk::DrawEvent&) override;
            void mouseMoveEvent(ftk::MouseMoveEvent&) override;
            void mousePressEvent(ftk::MouseClickEvent&) override;
            void mouseReleaseEvent(ftk::MouseClickEvent&) override;
            //void keyPressEvent(ftk::KeyEvent&) override;
            //void keyReleaseEvent(ftk::KeyEvent&) override;

        protected:
            void _timeUnitsUpdate() override;

        private:
            bool _isTrackVisible(int) const;

            ftk::Size2I _getLabelMaxSize(
                const std::shared_ptr<ftk::FontSystem>&) const;
            void _getTimeTicks(
                const std::shared_ptr<ftk::FontSystem>&,
                double& seconds,
                int& tick);

            void _drawInOutPoints(
                const ftk::Box2I&,
                const ftk::DrawEvent&);
            void _drawFrameMarkers(
                const ftk::Box2I&,
                const ftk::DrawEvent&);
            void _drawCacheInfo(
                const ftk::Box2I&,
                const ftk::DrawEvent&);
            void _drawTimeLabels(
                const ftk::Box2I&,
                const ftk::DrawEvent&);
            void _drawTimeTicks(
                const ftk::Box2I&,
                const ftk::DrawEvent&);
            void _drawCurrentTime(
                const ftk::Box2I&,
                const ftk::DrawEvent&);

            void _tracksUpdate();
            void _textUpdate();

            FTK_PRIVATE();
        };
    }
}
