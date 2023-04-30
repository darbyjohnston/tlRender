// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2023 Darby Johnston
// All rights reserved.

#pragma once

#include <tlUI/IWidget.h>
#include <tlUI/FloatModel.h>

namespace tl
{
    namespace ui
    {
        //! Floating point value slider.
        class FloatSlider : public IWidget
        {
            TLRENDER_NON_COPYABLE(FloatSlider);

        protected:
            void _init(
                const std::shared_ptr<system::Context>&,
                const std::shared_ptr<IWidget>& parent = nullptr);

            FloatSlider();

        public:
            ~FloatSlider() override;

            //! Create a new floating point value slider.
            static std::shared_ptr<FloatSlider> create(
                const std::shared_ptr<system::Context>&,
                const std::shared_ptr<IWidget>& parent = nullptr);

            //! Get the floating point model.
            const std::shared_ptr<FloatModel>& getModel() const;

            //! Set the floating point model.
            void setModel(const std::shared_ptr<FloatModel>&);

            bool acceptsKeyFocus() const override;
            void sizeHintEvent(const SizeHintEvent&) override;
            void drawEvent(const DrawEvent&) override;
            void enterEvent() override;
            void leaveEvent() override;
            void mouseMoveEvent(MouseMoveEvent&) override;
            void mousePressEvent(MouseClickEvent&) override;
            void mouseReleaseEvent(MouseClickEvent&) override;
            void keyPressEvent(KeyEvent&) override;
            void keyReleaseEvent(KeyEvent&) override;

        private:
            math::BBox2i _getSliderGeometry() const;

            float _posToValue(int) const;
            int _valueToPos(float) const;

            TLRENDER_PRIVATE();
        };
    }
}
