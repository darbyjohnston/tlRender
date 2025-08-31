// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2025 Darby Johnston
// All rights reserved.

#pragma once

#include <tlQtWidget/ContainerWidget.h>

#include <tlTimelineUI/IItem.h>

#include <tlTimeline/Player.h>

namespace tl
{
    namespace timeline
    {
        class Player;
    }

    namespace qtwidget
    {
        //! Timeline widget.
        class TimelineWidget : public qtwidget::ContainerWidget
        {
            Q_OBJECT

        public:
            TimelineWidget(
                const std::shared_ptr<ftk::Context>&,
                const std::shared_ptr<timeline::ITimeUnitsModel>&,
                const std::shared_ptr<ftk::Style>&,
                QWidget* parent = nullptr);

            virtual ~TimelineWidget();

            //! Get the timeline player.
            std::shared_ptr<timeline::Player>& player() const;

            //! Set the timeline player.
            void setPlayer(const std::shared_ptr<timeline::Player>&);

            //! Get whether the view is framed automatically.
            bool hasFrameView() const;

            //! Get whether the scroll bars are visible.
            bool areScrollBarsVisible() const;

            //! Get whether auto-scroll is enabled.
            bool hasAutoScroll() const;

            //! Get whether to stop playback when scrubbing.
            bool hasStopOnScrub() const;

            //! Get the frame markers.
            const std::vector<int>& frameMarkers() const;

            //! Get the item options.
            const timelineui::ItemOptions& itemOptions() const;

            //! Get the display options.
            const timelineui::DisplayOptions& displayOptions() const;

        public Q_SLOTS:
            //! Set whether the view is framed automatically.
            void setFrameView(bool);

            //! Set whether the scroll bars are visible.
            void setScrollBarsVisible(bool);

            //! Set whether auto-scroll is enabled.
            void setAutoScroll(bool);

            //! Set the scroll binding.
            void setScrollBinding(int button, ftk::KeyModifier);

            //! Set the mouse wheel scale.
            void setMouseWheelScale(float);

            //! Set whether to stop playback when scrubbing.
            void setStopOnScrub(bool);

            //! Set the frame markers.
            void setFrameMarkers(const std::vector<int>&);

            //! Set the item options.
            void setItemOptions(const timelineui::ItemOptions&);

            //! Set the display options.
            void setDisplayOptions(const timelineui::DisplayOptions&);

        Q_SIGNALS:
            //! This signal is emitted when the frame view is changed.
            void frameViewChanged(bool);

            //! This signal is emitted when scrubbing is in progress.
            void scrubChanged(bool);

            //! This signal is emitted when the time is scrubbed.
            void timeScrubbed(const OTIO_NS::RationalTime&);

        private:
            FTK_PRIVATE();
        };
    }
}
