// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2023 Darby Johnston
// All rights reserved.

#pragma once

#include <tlTimelineUI/IBasicItem.h>

#include <opentimelineio/transition.h>

namespace tl
{
    namespace timelineui
    {
        //! Transition item.
        class TransitionItem : public IBasicItem
        {
        protected:
            void _init(
                const otio::SerializableObject::Retainer<otio::Transition>&,
                const ItemData&,
                const std::shared_ptr<system::Context>&,
                const std::shared_ptr<IWidget>& parent);

            TransitionItem();

        public:
            ~TransitionItem() override;

            //! Create a new item.
            static std::shared_ptr<TransitionItem> create(
                const otio::SerializableObject::Retainer<otio::Transition>&,
                const ItemData&,
                const std::shared_ptr<system::Context>&,
                const std::shared_ptr<IWidget>& parent = nullptr);
        };
    }
}
