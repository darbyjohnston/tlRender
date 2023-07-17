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
        //! Floating point value slider.
        class FloatSlider : public IWidget
        {
            TLRENDER_NON_COPYABLE(FloatSlider);

        protected:
            void _init(
                const std::shared_ptr<system::Context>&,
                const std::shared_ptr<FloatModel>&,
                const std::shared_ptr<IWidget>& parent);

            FloatSlider();

        public:
            ~FloatSlider() override;

            //! Create a new widget.
            static std::shared_ptr<FloatSlider> create(
                const std::shared_ptr<system::Context>&,
                const std::shared_ptr<FloatModel>& = nullptr,
                const std::shared_ptr<IWidget>& parent = nullptr);

            //! Get the model.
            const std::shared_ptr<FloatModel>& getModel() const;

            void setVisible(bool) override;
            void setEnabled(bool) override;
            void sizeHintEvent(const SizeHintEvent&) override;
            void clipEvent(
                const math::BBox2i&,
                bool,
                const ClipEvent&) override;
            void drawEvent(
                const math::BBox2i&,
                const DrawEvent&) override;
            void mouseEnterEvent() override;
            void mouseLeaveEvent() override;
            void mouseMoveEvent(MouseMoveEvent&) override;
            void mousePressEvent(MouseClickEvent&) override;
            void mouseReleaseEvent(MouseClickEvent&) override;
            void keyPressEvent(KeyEvent&) override;
            void keyReleaseEvent(KeyEvent&) override;

        private:
            math::BBox2i _getSliderGeometry() const;

            float _posToValue(int) const;
            int _valueToPos(float) const;

            void _resetMouse();

            TLRENDER_PRIVATE();
        };
    }
}
