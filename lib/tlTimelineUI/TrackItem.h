// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2023 Darby Johnston
// All rights reserved.

#pragma once

#include <tlTimelineUI/IItem.h>

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
                const otio::SerializableObject::Retainer<otio::Track>&,
                int,
                const ItemData&,
                const std::shared_ptr<system::Context>&,
                const std::shared_ptr<IWidget>& parent = nullptr);

            TrackItem();

        public:
            ~TrackItem() override;

            //! Create a new item.
            static std::shared_ptr<TrackItem> create(
                const otio::SerializableObject::Retainer<otio::Track>&,
                int,
                const ItemData&,
                const std::shared_ptr<system::Context>&,
                const std::shared_ptr<IWidget>& parent = nullptr);

            void setOptions(const ItemOptions&) override;

            void setGeometry(const math::BBox2i&) override;
            void sizeHintEvent(const ui::SizeHintEvent&) override;
            void drawEvent(
                const math::BBox2i&,
                const ui::DrawEvent&) override;

        protected:
            void _timeUnitsUpdate() override;

        private:
            void _textUpdate();
            void _transitionsUpdate();

            TLRENDER_PRIVATE();
        };
    }
}
