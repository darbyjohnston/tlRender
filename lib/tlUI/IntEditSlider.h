// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2023 Darby Johnston
// All rights reserved.

#pragma once

#include <tlUI/IWidget.h>
#include <tlUI/IntModel.h>

namespace tl
{
    namespace ui
    {
        //! Integer value editor and slider.
        class IntEditSlider : public IWidget
        {
            TLRENDER_NON_COPYABLE(IntEditSlider);

        protected:
            void _init(
                const std::shared_ptr<system::Context>&,
                const std::shared_ptr<IntModel>&,
                const std::shared_ptr<IWidget>& parent);

            IntEditSlider();

        public:
            ~IntEditSlider() override;

            //! Create a new widget.
            static std::shared_ptr<IntEditSlider> create(
                const std::shared_ptr<system::Context>&,
                const std::shared_ptr<IntModel>& = nullptr,
                const std::shared_ptr<IWidget>& parent = nullptr);

            //! Get the model.
            const std::shared_ptr<IntModel>& getModel() const;

            //! Set the number of digits to display.
            void setDigits(int);

            //! Set the font role.
            void setFontRole(FontRole);

            void setGeometry(const math::BBox2i&) override;
            void sizeHintEvent(const SizeHintEvent&) override;

        private:
            TLRENDER_PRIVATE();
        };
    }
}
