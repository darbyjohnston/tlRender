// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2025 Darby Johnston
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
                const std::shared_ptr<feather_tk::Context>&,
                const std::string& label,
                feather_tk::ColorRole,
                const std::string& objectName,
                const OTIO_NS::SerializableObject::Retainer<OTIO_NS::Item>&,
                double scale,
                const ItemOptions&,
                const DisplayOptions&,
                const std::shared_ptr<ItemData>&,
                const std::shared_ptr<feather_tk::IWidget>& parent = nullptr);

            IBasicItem();

        public:
            virtual ~IBasicItem() = 0;

            void setScale(double) override;
            void setDisplayOptions(const DisplayOptions&) override;

            void setGeometry(const feather_tk::Box2I&) override;
            void sizeHintEvent(const feather_tk::SizeHintEvent&) override;
            void clipEvent(const feather_tk::Box2I&, bool) override;
            void drawEvent(const feather_tk::Box2I&, const feather_tk::DrawEvent&) override;

        protected:
            int _getMargin() const;
            int _getLineHeight() const;
            feather_tk::Box2I _getInsideGeometry() const;

            void _timeUnitsUpdate() override;

        private:
            void _textUpdate();

            FEATHER_TK_PRIVATE();
        };
    }
}
