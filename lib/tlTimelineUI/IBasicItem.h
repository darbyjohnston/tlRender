// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2023 Darby Johnston
// All rights reserved.

#pragma once

#include <tlTimelineUI/IItem.h>

#include <opentimelineio/gap.h>

namespace tl
{
    namespace timelineui
    {
        //! Base class for clips, gaps, and other items.
        class IBasicItem : public IItem
        {
        protected:
            void _init(
                const otime::TimeRange&,
                const std::string& label,
                const imaging::Color4f&,
                const std::vector<Marker>&,
                const std::string& name,
                const ItemData&,
                const std::shared_ptr<system::Context>&,
                const std::shared_ptr<IWidget>& parent = nullptr);

            IBasicItem();

        public:
            ~IBasicItem() override = 0;

            void sizeHintEvent(const ui::SizeHintEvent&) override;
            void clipEvent(
                const math::BBox2i&,
                bool,
                const ui::ClipEvent&) override;
            void drawEvent(
                const math::BBox2i&,
                const ui::DrawEvent&) override;

        protected:
            math::BBox2i _getInsideGeometry() const;
            int _getLineHeight() const;

            void _timeUnitsUpdate() override;

        private:
            void _textUpdate();

            TLRENDER_PRIVATE();
        };
    }
}
