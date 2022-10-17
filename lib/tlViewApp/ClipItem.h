// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2022 Darby Johnston
// All rights reserved.

#pragma once

#include <tlViewApp/IGraphicsItem.h>

#include <opentimelineio/clip.h>

namespace tl
{
    namespace view
    {
        //! Clip item.
        class ClipItem : public IGraphicsItem
        {
            TLRENDER_NON_COPYABLE(ClipItem);

        protected:
            void _init(
                otio::Clip*,
                const std::shared_ptr<IGraphicsItem>& parent = nullptr);

            ClipItem();

        public:

            ~ClipItem() override;

            static std::shared_ptr<ClipItem> create(
                otio::Clip*,
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
