// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2025 Darby Johnston
// All rights reserved.

#pragma once

#include <tlTimelineUI/IBasicItem.h>

#include <opentimelineio/transition.h>

namespace tl
{
    namespace timelineui
    {
        //! Transition item.
        class TransitionItem : public IItem
        {
        protected:
            void _init(
                const std::shared_ptr<feather_tk::Context>&,
                const OTIO_NS::SerializableObject::Retainer<OTIO_NS::Transition>&,
                double scale,
                const ItemOptions&,
                const DisplayOptions&,
                const std::shared_ptr<ItemData>&,
                const std::shared_ptr<IWidget>& parent);

            TransitionItem();

        public:
            virtual ~TransitionItem();

            //! Create a new item.
            static std::shared_ptr<TransitionItem> create(
                const std::shared_ptr<feather_tk::Context>&,
                const OTIO_NS::SerializableObject::Retainer<OTIO_NS::Transition>&,
                double scale,
                const ItemOptions&,
                const DisplayOptions&,
                const std::shared_ptr<ItemData>&,
                const std::shared_ptr<IWidget>& parent = nullptr);
        };
    }
}
