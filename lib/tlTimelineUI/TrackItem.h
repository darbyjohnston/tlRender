// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2023 Darby Johnston
// All rights reserved.

#pragma once

#include <tlTimelineUI/IItem.h>

#include <tlTimeline/Player.h>

#include <opentimelineio/track.h>

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

        //! Track item.
        class TrackItem : public IItem
        {
        protected:
            void _init(
                const std::shared_ptr<timeline::Player>&,
                const otio::SerializableObject::Retainer<otio::Track>&,
                int trackIndex,
                const otime::TimeRange&,
                const ItemData&,
                const std::shared_ptr<system::Context>&,
                const std::shared_ptr<IWidget>& parent);

            TrackItem();

        public:
            virtual ~TrackItem();

            //! Create a new item.
            static std::shared_ptr<TrackItem> create(
                const std::shared_ptr<timeline::Player>&,
                const otio::SerializableObject::Retainer<otio::Track>&,
                int trackIndex,
                const otime::TimeRange&,
                const ItemData&,
                const std::shared_ptr<system::Context>&,
                const std::shared_ptr<IWidget>& parent = nullptr);

            void setOptions(const ItemOptions&) override;

            void setGeometry(const math::Box2i&) override;
            void sizeHintEvent(const ui::SizeHintEvent&) override;
            void drawEvent(
                const math::Box2i&,
                const ui::DrawEvent&) override;
            void dragEnterEvent(ui::DragAndDropEvent&) override;
            void dragLeaveEvent(ui::DragAndDropEvent&) override;
            void dragMoveEvent(ui::DragAndDropEvent&) override;
            void dropEvent(ui::DragAndDropEvent&) override;

        protected:
            void _timeUnitsUpdate() override;

        private:
            void _textUpdate();
            void _transitionsUpdate();

            TLRENDER_PRIVATE();
        };
    }
}
