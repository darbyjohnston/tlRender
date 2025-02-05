// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2025 Darby Johnston
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
        //! Time value editor.
        class TimeEdit : public IWidget
        {
            TLRENDER_NON_COPYABLE(TimeEdit);

        protected:
            void _init(
                const std::shared_ptr<timeline::TimeUnitsModel>&,
                const std::shared_ptr<dtk::Context>&,
                const std::shared_ptr<IWidget>& parent);

            TimeEdit();

        public:
            virtual ~TimeEdit();

            //! Create a new widget.
            static std::shared_ptr<TimeEdit> create(
                const std::shared_ptr<timeline::TimeUnitsModel>&,
                const std::shared_ptr<dtk::Context>&,
                const std::shared_ptr<IWidget>& parent = nullptr);

            //! Get the time units model.
            const std::shared_ptr<timeline::TimeUnitsModel>& getTimeUnitsModel() const;

            //! Get the time value.
            const OTIO_NS::RationalTime& getValue() const;

            //! Set the time value.
            void setValue(const OTIO_NS::RationalTime&);

            //! Set the time value callback.
            void setCallback(const std::function<void(const OTIO_NS::RationalTime&)>&);

            //! Set the font role.
            void setFontRole(FontRole);

            void setGeometry(const dtk::Box2I&) override;
            void takeKeyFocus() override;
            void sizeHintEvent(const SizeHintEvent&) override;
            void keyPressEvent(KeyEvent&) override;
            void keyReleaseEvent(KeyEvent&) override;

        private:
            void _commitValue(const std::string&);
            void _commitValue(const OTIO_NS::RationalTime&);
            void _textUpdate();

            TLRENDER_PRIVATE();
        };
    }
}
