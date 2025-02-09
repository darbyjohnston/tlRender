// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2025 Darby Johnston
// All rights reserved.

#pragma once

#include <dtk/ui/IWidget.h>

#include <opentimelineio/version.h>

namespace tl
{
    namespace timeline
    {
        class TimeUnitsModel;
    }

    namespace timelineui
    {
        //! Time label.
        class TimeLabel : public dtk::IWidget
        {
            DTK_NON_COPYABLE(TimeLabel);

        protected:
            void _init(
                const std::shared_ptr<dtk::Context>&,
                const std::shared_ptr<timeline::TimeUnitsModel>&,
                const std::shared_ptr<IWidget>& parent);

            TimeLabel();

        public:
            virtual ~TimeLabel();

            //! Create a new widget.
            static std::shared_ptr<TimeLabel> create(
                const std::shared_ptr<dtk::Context>&,
                const std::shared_ptr<timeline::TimeUnitsModel>&,
                const std::shared_ptr<IWidget>& parent = nullptr);

            //! Get the time units model.
            const std::shared_ptr<timeline::TimeUnitsModel>& getTimeUnitsModel() const;

            //! Get the time value.
            const OTIO_NS::RationalTime& getValue() const;

            //! Set the time value.
            void setValue(const OTIO_NS::RationalTime&);

            //! Set the margin role.
            void setMarginRole(dtk::SizeRole);

            //! Set the font role.
            void setFontRole(dtk::FontRole);

            void sizeHintEvent(const dtk::SizeHintEvent&) override;
            void clipEvent(const dtk::Box2I&, bool) override;
            void drawEvent(const dtk::Box2I&, const dtk::DrawEvent&) override;

        private:
            void _textUpdate();

            DTK_PRIVATE();
        };
    }
}
