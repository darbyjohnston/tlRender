// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2022 Darby Johnston
// All rights reserved.

#pragma once

#include <tlViewApp/IGraphicsItem.h>

#include <opentimelineio/timeline.h>

namespace tl
{
    namespace view
    {
        //! Timeline item.
        class TimelineItem : public IGraphicsItem
        {
            TLRENDER_NON_COPYABLE(TimelineItem);

        protected:
            void _init(
                otio::Timeline*,
                const std::shared_ptr<IGraphicsItem>& parent = nullptr);

            TimelineItem();

        public:
            ~TimelineItem() override;

            static std::shared_ptr<TimelineItem> create(
                otio::Timeline*,
                const std::shared_ptr<IGraphicsItem>& parent = nullptr);

            size_t getTrackCount() const;

            virtual math::Vector2i getSize(
                const std::shared_ptr<imaging::FontSystem>&) const override;
            void draw(
                const math::BBox2i&,
                const std::shared_ptr<imaging::FontSystem>&,
                const std::shared_ptr<timeline::IRender>&) override;

        private:
            TLRENDER_PRIVATE();
        };
    }
}
