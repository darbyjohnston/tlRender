// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2023 Darby Johnston
// All rights reserved.

#pragma once

#include <tlTimelineUI/IBasicItem.h>

#include <opentimelineio/clip.h>

namespace tl
{
    namespace ui
    {
        class ThumbnailGenerator;
    }
    
    namespace timelineui
    {
        //! Video clip item.
        class VideoClipItem : public IBasicItem
        {
        protected:
            void _init(
                const otio::SerializableObject::Retainer<otio::Clip>&,
                double scale,
                const ItemOptions&,
                const std::shared_ptr<ItemData>&,
                const std::shared_ptr<ui::ThumbnailGenerator>,
                const std::shared_ptr<system::Context>&,
                const std::shared_ptr<IWidget>& parent);

            VideoClipItem();

        public:
            virtual ~VideoClipItem();

            //! Create a new item.
            static std::shared_ptr<VideoClipItem> create(
                const otio::SerializableObject::Retainer<otio::Clip>&,
                double scale,
                const ItemOptions&,
                const std::shared_ptr<ItemData>&,
                const std::shared_ptr<ui::ThumbnailGenerator>,
                const std::shared_ptr<system::Context>&,
                const std::shared_ptr<IWidget>& parent = nullptr);

            void setScale(double) override;
            void setOptions(const ItemOptions&) override;

            void tickEvent(
                bool,
                bool,
                const ui::TickEvent&) override;
            void sizeHintEvent(const ui::SizeHintEvent&) override;
            void clipEvent(const math::Box2i&, bool) override;
            void drawEvent(const math::Box2i&, const ui::DrawEvent&) override;

        private:
            std::string _getThumbnailKey(const otime::RationalTime&) const;

            void _drawInfo(
                const math::Box2i&,
                const ui::DrawEvent&);
            void _drawThumbnails(
                const math::Box2i&,
                const ui::DrawEvent&);

            void _cancelRequests();

            TLRENDER_PRIVATE();
        };
    }
}
