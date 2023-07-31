// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2023 Darby Johnston
// All rights reserved.

#pragma once

#include <tlUI/DoubleModel.h>
#include <tlUI/IWidget.h>

namespace tl
{
    namespace ui
    {
        //! Double precision floating point slider.
        class DoubleSlider : public IWidget
        {
            TLRENDER_NON_COPYABLE(DoubleSlider);

        protected:
            void _init(
                const std::shared_ptr<system::Context>&,
                const std::shared_ptr<DoubleModel>&,
                const std::shared_ptr<IWidget>& parent);

            DoubleSlider();

        public:
            virtual ~DoubleSlider();

            //! Create a new widget.
            static std::shared_ptr<DoubleSlider> create(
                const std::shared_ptr<system::Context>&,
                const std::shared_ptr<DoubleModel>& = nullptr,
                const std::shared_ptr<IWidget>& parent = nullptr);

            //! Get the model.
            const std::shared_ptr<DoubleModel>& getModel() const;

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

            double _posToValue(int) const;
            int _valueToPos(double) const;

            void _resetMouse();

            TLRENDER_PRIVATE();
        };
    }
}
