// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2022 Darby Johnston
// All rights reserved.

#pragma once

#include <tlViewApp/IGraphicsItem.h>

#include <opentimelineio/stack.h>

namespace tl
{
    namespace view
    {
        //! Stack item.
        class StackItem : public IGraphicsItem
        {
            TLRENDER_NON_COPYABLE(StackItem);

        protected:
            void _init(
                otio::Stack*,
                const std::shared_ptr<IGraphicsItem>& parent = nullptr);

            StackItem();

        public:
            ~StackItem() override;

            static std::shared_ptr<StackItem> create(
                otio::Stack*,
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
