// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2025 Darby Johnston
// All rights reserved.

#pragma once

#include <tlUI/FloatModel.h>
#include <tlUI/IWidget.h>

namespace tl
{
    namespace ui
    {
        //! Floating point value editor and slider.
        class FloatEditSlider : public IWidget
        {
            DTK_NON_COPYABLE(FloatEditSlider);

        protected:
            void _init(
                const std::shared_ptr<dtk::Context>&,
                const std::shared_ptr<FloatModel>&,
                const std::shared_ptr<IWidget>& parent);

            FloatEditSlider();

        public:
            virtual ~FloatEditSlider();

            //! Create a new widget.
            static std::shared_ptr<FloatEditSlider> create(
                const std::shared_ptr<dtk::Context>&,
                const std::shared_ptr<FloatModel>& = nullptr,
                const std::shared_ptr<IWidget>& parent = nullptr);

            //! Get the value.
            float getValue() const;

            //! Set the value.
            void setValue(float);

            //! Set the callback.
            void setCallback(const std::function<void(float)>&);

            //! Get the range.
            const dtk::RangeF& getRange() const;

            //! Set the range.
            void setRange(const dtk::RangeF&);

            //! Set the step.
            void setStep(float);

            //! Set the large step.
            void setLargeStep(float);

            //! Set the default value.
            void setDefaultValue(float);

            //! Get the model.
            const std::shared_ptr<FloatModel>& getModel() const;

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
