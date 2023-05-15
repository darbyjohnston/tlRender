// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2023 Darby Johnston
// All rights reserved.

#pragma once

#include <tlUI/IWidget.h>

namespace tl
{
    namespace ui
    {
        class TimeUnitsModel;

        //! Time value editor.
        class TimeEdit : public IWidget
        {
            TLRENDER_NON_COPYABLE(TimeEdit);

        protected:
            void _init(
                const std::shared_ptr<TimeUnitsModel>&,
                const std::shared_ptr<system::Context>&,
                const std::shared_ptr<IWidget>& parent = nullptr);

            TimeEdit();

        public:
            ~TimeEdit() override;

            //! Create a new widget.
            static std::shared_ptr<TimeEdit> create(
                const std::shared_ptr<TimeUnitsModel>&,
                const std::shared_ptr<system::Context>&,
                const std::shared_ptr<IWidget>& parent = nullptr);

            //! Get the time units model.
            const std::shared_ptr<TimeUnitsModel>& getTimeUnitsModel() const;

            //! Get the time value.
            const otime::RationalTime& getTime() const;

            //! Set the time value.
            void setTime(const otime::RationalTime&);

            //! Set the time value callback.
            void setTimeCallback(const std::function<void(const otime::RationalTime&)>&);

            //! Set the font role.
            void setFontRole(FontRole);

            void setGeometry(const math::BBox2i&) override;
            void sizeHintEvent(const SizeHintEvent&) override;
            void keyPressEvent(KeyEvent&) override;
            void keyReleaseEvent(KeyEvent&) override;

        private:
            void _textUpdate();

            TLRENDER_PRIVATE();
        };
    }
}
