// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2022 Darby Johnston
// All rights reserved.

#pragma once

#include <tlViewApp/IGraphicsItem.h>

#include <opentimelineio/track.h>

namespace tl
{
    namespace view
    {
        //! Track item.
        class TrackItem : public IGraphicsItem
        {
            TLRENDER_NON_COPYABLE(TrackItem);

        protected:
            void _init(
                otio::Track*,
                const std::shared_ptr<IGraphicsItem>& parent = nullptr);

            TrackItem();

        public:
            ~TrackItem() override;

            static std::shared_ptr<TrackItem> create(
                otio::Track*,
                const std::shared_ptr<IGraphicsItem>& parent = nullptr);

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
