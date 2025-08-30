// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2025 Darby Johnston
// All rights reserved.

#pragma once

#include <tlTimelineUI/IBasicItem.h>

#include <opentimelineio/clip.h>

namespace tl
{
    namespace timelineui
    {
        class ThumbnailGenerator;

        //! Video clip item.
        class VideoClipItem : public IBasicItem
        {
        protected:
            void _init(
                const std::shared_ptr<ftk::Context>&,
                const OTIO_NS::SerializableObject::Retainer<OTIO_NS::Clip>&,
                double scale,
                const ItemOptions&,
                const DisplayOptions&,
                const std::shared_ptr<ItemData>&,
                const std::shared_ptr<ThumbnailGenerator>,
                const std::shared_ptr<IWidget>& parent);

            VideoClipItem();

        public:
            virtual ~VideoClipItem();

            //! Create a new item.
            static std::shared_ptr<VideoClipItem> create(
                const std::shared_ptr<ftk::Context>&,
                const OTIO_NS::SerializableObject::Retainer<OTIO_NS::Clip>&,
                double scale,
                const ItemOptions&,
                const DisplayOptions&,
                const std::shared_ptr<ItemData>&,
                const std::shared_ptr<ThumbnailGenerator>,
                const std::shared_ptr<IWidget>& parent = nullptr);

            void setScale(double) override;
            void setDisplayOptions(const DisplayOptions&) override;

            void tickEvent(
                bool,
                bool,
                const ftk::TickEvent&) override;
            void sizeHintEvent(const ftk::SizeHintEvent&) override;
            void clipEvent(const ftk::Box2I&, bool) override;
            void drawEvent(const ftk::Box2I&, const ftk::DrawEvent&) override;

        private:
            void _drawThumbnails(
                const ftk::Box2I&,
                const ftk::DrawEvent&);

            void _cancelRequests();

            FTK_PRIVATE();
        };
    }
}
