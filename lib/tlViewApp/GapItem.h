// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2022 Darby Johnston
// All rights reserved.

#pragma once

#include <tlViewApp/IGraphicsItem.h>

#include <opentimelineio/gap.h>

namespace tl
{
    namespace view
    {
        //! Gap item.
        class GapItem : public IGraphicsItem
        {
            TLRENDER_NON_COPYABLE(GapItem);

        protected:
            void _init(
                otio::Gap*,
                const std::shared_ptr<IGraphicsItem>& parent = nullptr);

            GapItem();

        public:
            ~GapItem() override;

            static std::shared_ptr<GapItem> create(
                otio::Gap*,
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
