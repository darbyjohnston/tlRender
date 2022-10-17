// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2022 Darby Johnston
// All rights reserved.

#pragma once

#include <tlViewApp/IGraphicsItem.h>

#include <opentimelineio/transition.h>

namespace tl
{
    namespace view
    {
        //! Transition item.
        class TransitionItem : public IGraphicsItem
        {
            TLRENDER_NON_COPYABLE(TransitionItem);

        protected:
            void _init(
                otio::Transition*,
                const std::shared_ptr<IGraphicsItem>& parent = nullptr);

            TransitionItem();

        public:
            ~TransitionItem() override;

            static std::shared_ptr<TransitionItem> create(
                otio::Transition*,
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
