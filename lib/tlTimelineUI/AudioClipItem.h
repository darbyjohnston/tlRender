// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2023 Darby Johnston
// All rights reserved.

#pragma once

#include <tlTimelineUI/IBasicItem.h>

#include <opentimelineio/clip.h>

namespace tl
{
    namespace timelineui
    {
        class AudioClipItem;

        //! Audio drag and drop data.
        class AudioDragAndDropData : public ui::DragAndDropData
        {
        public:
            AudioDragAndDropData(const std::shared_ptr<AudioClipItem>&);

            virtual ~AudioDragAndDropData();

            const std::shared_ptr<AudioClipItem>& getItem() const;

        private:
            std::shared_ptr<AudioClipItem> _item;
        };

        //! Audio clip item.
        class AudioClipItem : public IBasicItem
        {
        protected:
            void _init(
                const otio::SerializableObject::Retainer<otio::Clip>&,
                const otime::TimeRange&,
                const ItemData&,
                const std::shared_ptr<system::Context>&,
                const std::shared_ptr<IWidget>& parent);

            AudioClipItem();

        public:
            virtual ~AudioClipItem();

            //! Create a new item.
            static std::shared_ptr<AudioClipItem> create(
                const otio::SerializableObject::Retainer<otio::Clip>&,
                const otime::TimeRange&,
                const ItemData&,
                const std::shared_ptr<system::Context>&,
                const std::shared_ptr<IWidget>& parent = nullptr);

            void setScale(double) override;
            void setOptions(const ItemOptions&) override;

            void tickEvent(
                bool,
                bool,
                const ui::TickEvent&) override;
            void sizeHintEvent(const ui::SizeHintEvent&) override;
            void clipEvent(
                const math::Box2i&,
                bool,
                const ui::ClipEvent&) override;
            void drawEvent(
                const math::Box2i&,
                const ui::DrawEvent&) override;
            void mouseMoveEvent(ui::MouseMoveEvent&) override;

        private:
            void _drawWaveforms(
                const math::Box2i&,
                const ui::DrawEvent&);

            TLRENDER_PRIVATE();
        };
    }
}
