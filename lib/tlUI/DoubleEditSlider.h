// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2025 Darby Johnston
// All rights reserved.

#pragma once

#include <tlUI/DoubleModel.h>
#include <tlUI/IWidget.h>

namespace tl
{
    namespace ui
    {
        //! Double precision floating point value editor and slider.
        class DoubleEditSlider : public IWidget
        {
            DTK_NON_COPYABLE(DoubleEditSlider);

        protected:
            void _init(
                const std::shared_ptr<dtk::Context>&,
                const std::shared_ptr<DoubleModel>&,
                const std::shared_ptr<IWidget>& parent);

            DoubleEditSlider();

        public:
            virtual ~DoubleEditSlider();

            //! Create a new widget.
            static std::shared_ptr<DoubleEditSlider> create(
                const std::shared_ptr<dtk::Context>&,
                const std::shared_ptr<DoubleModel>& = nullptr,
                const std::shared_ptr<IWidget>& parent = nullptr);

            //! Get the value.
            double getValue() const;

            //! Set the value.
            void setValue(double);

            //! Set the callback.
            void setCallback(const std::function<void(double)>&);

            //! Get the range.
            const dtk::RangeD& getRange() const;

            //! Set the range.
            void setRange(const dtk::RangeD&);

            //! Set the step.
            void setStep(double);

            //! Set the large step.
            void setLargeStep(double);

            //! Set the default value.
            void setDefaultValue(double);

            //! Get the model.
            const std::shared_ptr<DoubleModel>& getModel() const;

            //! Set the display precision.
            void setPrecision(int);

            //! Set the font role.
            void setFontRole(FontRole);

            void setGeometry(const dtk::Box2I&) override;
            void sizeHintEvent(const SizeHintEvent&) override;

        private:
            DTK_PRIVATE();
        };
    }
}
