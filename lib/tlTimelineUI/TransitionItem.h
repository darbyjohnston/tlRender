// SPDX-License-Identifier: BSD-3-Clause
// Copyright Contributors to the tlRender project.

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
                const std::shared_ptr<ftk::Context>&,
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
                const std::shared_ptr<ftk::Context>&,
                const OTIO_NS::SerializableObject::Retainer<OTIO_NS::Transition>&,
                double scale,
                const ItemOptions&,
                const DisplayOptions&,
                const std::shared_ptr<ItemData>&,
                const std::shared_ptr<IWidget>& parent = nullptr);
        };
    }
}
