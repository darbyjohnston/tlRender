// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2023 Darby Johnston
// All rights reserved.

#pragma once

#include <tlTimelineUI/IItem.h>

#include <opentimelineio/gap.h>

namespace tl
{
    namespace timelineui
    {
        //! Video gap item.
        class VideoGapItem : public IItem
        {
        protected:
            void _init(
                const otio::Gap*,
                const ItemData&,
                const std::shared_ptr<system::Context>&,
                const std::shared_ptr<IWidget>& parent = nullptr);

            VideoGapItem();

        public:
            ~VideoGapItem() override;

            //! Create a new item.
            static std::shared_ptr<VideoGapItem> create(
                const otio::Gap*,
                const ItemData&,
                const std::shared_ptr<system::Context>&,
                const std::shared_ptr<IWidget>& parent = nullptr);

            void setOptions(const ItemOptions&) override;

            void sizeHintEvent(const ui::SizeHintEvent&) override;
            void clipEvent(
                const math::BBox2i&,
                bool,
                const ui::ClipEvent&) override;
            void drawEvent(
                const math::BBox2i&,
                const ui::DrawEvent&) override;

        private:
            void _textUpdate();

            static std::string _nameLabel(const std::string&);

            TLRENDER_PRIVATE();
        };
    }
}
