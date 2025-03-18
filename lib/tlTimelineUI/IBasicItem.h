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
                const std::shared_ptr<dtk::Context>&,
                const std::string& label,
                dtk::ColorRole,
                const std::string& objectName,
                const OTIO_NS::SerializableObject::Retainer<OTIO_NS::Item>&,
                double scale,
                const ItemOptions&,
                const DisplayOptions&,
                const std::shared_ptr<ItemData>&,
                const std::shared_ptr<dtk::IWidget>& parent = nullptr);

            IBasicItem();

        public:
            virtual ~IBasicItem() = 0;

            void setScale(double) override;
            void setDisplayOptions(const DisplayOptions&) override;

            void setGeometry(const dtk::Box2I&) override;
            void sizeHintEvent(const dtk::SizeHintEvent&) override;
            void clipEvent(const dtk::Box2I&, bool) override;
            void drawEvent(const dtk::Box2I&, const dtk::DrawEvent&) override;

        protected:
            int _getMargin() const;
            int _getLineHeight() const;
            dtk::Box2I _getInsideGeometry() const;

            void _timeUnitsUpdate() override;

        private:
            void _textUpdate();

            DTK_PRIVATE();
        };
    }
}
