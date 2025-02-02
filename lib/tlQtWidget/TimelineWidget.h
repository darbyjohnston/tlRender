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
                const std::shared_ptr<timeline::ITimeUnitsModel>&,
                const std::shared_ptr<ui::Style>&,
                const std::shared_ptr<dtk::Context>&,
                QWidget* parent = nullptr);

            virtual ~TimelineWidget();

            //! Get the timeline player.
            std::shared_ptr<timeline::Player>& player() const;

            //! Set the timeline player.
            void setPlayer(const std::shared_ptr<timeline::Player>&);

            //! Get whether the timeline is editable.
            bool isEditable() const;

            //! Get whether the view is framed automatically.
            bool hasFrameView() const;

            //! Get whether the scroll bars are visible.
            bool areScrollBarsVisible() const;

            //! Get whether to automatically scroll to the current frame.
            bool hasScrollToCurrentFrame() const;

            //! Get the mouse scroll key modifier.
            ui::KeyModifier scrollKeyModifier() const;

            //! Get the mouse wheel scale.
            float mouseWheelScale() const;

            //! Get whether to stop playback when scrubbing.
            bool hasStopOnScrub() const;

            //! Get the frame markers.
            const std::vector<int>& frameMarkers() const;

            //! Get the item options.
            const timelineui::ItemOptions& itemOptions() const;

            //! Get the display options.
            const timelineui::DisplayOptions& displayOptions() const;

        public Q_SLOTS:
            //! Set whether the timeline is editable.
            void setEditable(bool);

            //! Set whether the view is framed automatically.
            void setFrameView(bool);

            //! Set whether the scroll bars are visible.
            void setScrollBarsVisible(bool);

            //! Set whether to automatically scroll to the current frame.
            void setScrollToCurrentFrame(bool);

            //! Set the mouse scroll key modifier.
            void setScrollKeyModifier(ui::KeyModifier);

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
            //! This signal is emitted when the editable timeline is changed.
            void editableChanged(bool);

            //! This signal is emitted when the frame view is changed.
            void frameViewChanged(bool);

            //! This signal is emitted when scrubbing is in progress.
            void scrubChanged(bool);

            //! This signal is emitted when the time is scrubbed.
            void timeScrubbed(const OTIO_NS::RationalTime&);

        protected:
            void contextMenuEvent(QContextMenuEvent* event) override;

        private Q_SLOTS:
            void _trackEnabledCallback(bool);

        private:
            TLRENDER_PRIVATE();
        };
    }
}
