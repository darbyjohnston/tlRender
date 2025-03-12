// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2025 Darby Johnston
// All rights reserved.

#pragma once

#include <tlTimelineUI/IBasicItem.h>

#include <opentimelineio/gap.h>

namespace tl
{
    namespace timelineui
    {
        //! Gap item.
        class GapItem : public IBasicItem
        {
        protected:
            void _init(
                const std::shared_ptr<dtk::Context>&,
                dtk::ColorRole,
                const OTIO_NS::SerializableObject::Retainer<OTIO_NS::Gap>&,
                double scale,
                const ItemOptions&,
                const DisplayOptions&,
                const std::shared_ptr<ItemData>&,
                const std::shared_ptr<IWidget>& parent);

            GapItem();

        public:
            virtual ~GapItem();

            //! Create a new item.
            static std::shared_ptr<GapItem> create(
                const std::shared_ptr<dtk::Context>&,
                dtk::ColorRole,
                const OTIO_NS::SerializableObject::Retainer<OTIO_NS::Gap>&,
                double scale,
                const ItemOptions&,
                const DisplayOptions&,
                const std::shared_ptr<ItemData>&,
                const std::shared_ptr<IWidget>& parent = nullptr);
        };
    }
}
