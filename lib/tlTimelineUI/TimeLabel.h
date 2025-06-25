// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2025 Darby Johnston
// All rights reserved.

#pragma once

#include <feather-tk/ui/IWidget.h>

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
        class TimeLabel : public feather_tk::IWidget
        {
            FEATHER_TK_NON_COPYABLE(TimeLabel);

        protected:
            void _init(
                const std::shared_ptr<feather_tk::Context>&,
                const std::shared_ptr<timeline::TimeUnitsModel>&,
                const std::shared_ptr<IWidget>& parent);

            TimeLabel();

        public:
            virtual ~TimeLabel();

            //! Create a new widget.
            static std::shared_ptr<TimeLabel> create(
                const std::shared_ptr<feather_tk::Context>&,
                const std::shared_ptr<timeline::TimeUnitsModel>&,
                const std::shared_ptr<IWidget>& parent = nullptr);

            //! Get the time units model.
            const std::shared_ptr<timeline::TimeUnitsModel>& getTimeUnitsModel() const;

            //! Get the time value.
            const OTIO_NS::RationalTime& getValue() const;

            //! Set the time value.
            void setValue(const OTIO_NS::RationalTime&);

            //! Set the margin role.
            void setMarginRole(feather_tk::SizeRole);

            //! Set the font role.
            void setFontRole(feather_tk::FontRole);

            void sizeHintEvent(const feather_tk::SizeHintEvent&) override;
            void clipEvent(const feather_tk::Box2I&, bool) override;
            void drawEvent(const feather_tk::Box2I&, const feather_tk::DrawEvent&) override;

        private:
            void _textUpdate();

            FEATHER_TK_PRIVATE();
        };
    }
}
