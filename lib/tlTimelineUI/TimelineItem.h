// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2025 Darby Johnston
// All rights reserved.

#pragma once

#include <tlTimelineUI/IItem.h>

#include <tlTimeline/Player.h>

namespace feather_tk
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
                const std::shared_ptr<feather_tk::Context>&,
                const std::shared_ptr<timeline::Player>&,
                const OTIO_NS::SerializableObject::Retainer<OTIO_NS::Stack>&,
                double scale,
                const ItemOptions&,
                const DisplayOptions&,
                const std::shared_ptr<ItemData>&,
                const std::shared_ptr<feather_tk::gl::Window>&,
                const std::shared_ptr<IWidget>& parent);

            TimelineItem();

        public:
            virtual ~TimelineItem();

            //! Create a new item.
            static std::shared_ptr<TimelineItem> create(
                const std::shared_ptr<feather_tk::Context>&,
                const std::shared_ptr<timeline::Player>&,
                const OTIO_NS::SerializableObject::Retainer<OTIO_NS::Stack>&,
                double scale,
                const ItemOptions&,
                const DisplayOptions&,
                const std::shared_ptr<ItemData>&,
                const std::shared_ptr<feather_tk::gl::Window>&,
                const std::shared_ptr<IWidget>& parent = nullptr);

            //! Set whether the timeline is editable.
            void setEditable(bool);

            //! Set whether playback stops when scrubbing.
            void setStopOnScrub(bool);

            //! Observe whether scrubbing is in progress.
            std::shared_ptr<feather_tk::IObservableValue<bool> > observeScrub() const;

            //! Observe time scrubbing.
            std::shared_ptr<feather_tk::IObservableValue<OTIO_NS::RationalTime> > observeTimeScrub() const;

            //! Set the frame markers.
            void setFrameMarkers(const std::vector<int>&);

            //! Get the track geometry.
            std::vector<feather_tk::Box2I> getTrackGeom() const;

            void setDisplayOptions(const DisplayOptions&) override;

            void setGeometry(const feather_tk::Box2I&) override;
            void sizeHintEvent(const feather_tk::SizeHintEvent&) override;
            void drawOverlayEvent(const feather_tk::Box2I&, const feather_tk::DrawEvent&) override;
            void mouseMoveEvent(feather_tk::MouseMoveEvent&) override;
            void mousePressEvent(feather_tk::MouseClickEvent&) override;
            void mouseReleaseEvent(feather_tk::MouseClickEvent&) override;
            //void keyPressEvent(feather_tk::KeyEvent&) override;
            //void keyReleaseEvent(feather_tk::KeyEvent&) override;

        protected:
            void _timeUnitsUpdate() override;

            void _releaseMouse() override;

        private:
            bool _isTrackVisible(int) const;
            void _setTrackEnabled(int, bool);

            void _drawInOutPoints(
                const feather_tk::Box2I&,
                const feather_tk::DrawEvent&);
            feather_tk::Size2I _getLabelMaxSize(
                const std::shared_ptr<feather_tk::FontSystem>&) const;
            void _getTimeTicks(
                const std::shared_ptr<feather_tk::FontSystem>&,
                double& seconds,
                int& tick);
            void _drawFrameMarkers(
                const feather_tk::Box2I&,
                const feather_tk::DrawEvent&);
            void _drawTimeLabels(
                const feather_tk::Box2I&,
                const feather_tk::DrawEvent&);
            void _drawCacheInfo(
                const feather_tk::Box2I&,
                const feather_tk::DrawEvent&);
            void _drawTimeTicks(
                const feather_tk::Box2I&,
                const feather_tk::DrawEvent&);
            void _drawCurrentTime(
                const feather_tk::Box2I&,
                const feather_tk::DrawEvent&);

            void _tracksUpdate();
            void _textUpdate();

            FEATHER_TK_PRIVATE();
        };
    }
}
