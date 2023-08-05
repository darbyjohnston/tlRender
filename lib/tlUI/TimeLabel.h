// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2023 Darby Johnston
// All rights reserved.

#pragma once

#include <tlUI/IWidget.h>

namespace tl
{
    namespace timeline
    {
        class TimeUnitsModel;
    }

    namespace ui
    {
        //! Time label.
        class TimeLabel : public IWidget
        {
            TLRENDER_NON_COPYABLE(TimeLabel);

        protected:
            void _init(
                const std::shared_ptr<timeline::TimeUnitsModel>&,
                const std::shared_ptr<system::Context>&,
                const std::shared_ptr<IWidget>& parent);

            TimeLabel();

        public:
            virtual ~TimeLabel();

            //! Create a new widget.
            static std::shared_ptr<TimeLabel> create(
                const std::shared_ptr<timeline::TimeUnitsModel>&,
                const std::shared_ptr<system::Context>&,
                const std::shared_ptr<IWidget>& parent = nullptr);

            //! Get the time units model.
            const std::shared_ptr<timeline::TimeUnitsModel>& getTimeUnitsModel() const;

            //! Get the time value.
            const otime::RationalTime& getValue() const;

            //! Set the time value.
            void setValue(const otime::RationalTime&);

            //! Set the margin role.
            void setMarginRole(SizeRole);

            //! Set the font role.
            void setFontRole(FontRole);

            void sizeHintEvent(const SizeHintEvent&) override;
            void clipEvent(
                const math::Box2i&,
                bool,
                const ClipEvent&) override;
            void drawEvent(
                const math::Box2i&,
                const DrawEvent&) override;

        private:
            void _textUpdate();

            TLRENDER_PRIVATE();
        };
    }
}
