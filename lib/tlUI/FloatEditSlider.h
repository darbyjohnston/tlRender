// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2023 Darby Johnston
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
            TLRENDER_NON_COPYABLE(FloatEditSlider);

        protected:
            void _init(
                const std::shared_ptr<system::Context>&,
                const std::shared_ptr<FloatModel>&,
                const std::shared_ptr<IWidget>& parent);

            FloatEditSlider();

        public:
            ~FloatEditSlider() override;

            //! Create a new widget.
            static std::shared_ptr<FloatEditSlider> create(
                const std::shared_ptr<system::Context>&,
                const std::shared_ptr<FloatModel>& = nullptr,
                const std::shared_ptr<IWidget>& parent = nullptr);

            //! Get the model.
            const std::shared_ptr<FloatModel>& getModel() const;

            //! Set the number of digits to display.
            void setDigits(int);

            //! Set the display precision.
            void setPrecision(int);

            //! Set the font role.
            void setFontRole(FontRole);

            void setGeometry(const math::BBox2i&) override;
            void sizeHintEvent(const SizeHintEvent&) override;

        private:
            TLRENDER_PRIVATE();
        };
    }
}
