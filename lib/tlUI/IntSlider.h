// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2023 Darby Johnston
// All rights reserved.

#pragma once

#include <tlUI/IWidget.h>

namespace tl
{
    namespace ui
    {
        class IntModel;

        //! Integer value slider.
        class IntSlider : public IWidget
        {
            TLRENDER_NON_COPYABLE(IntSlider);

        protected:
            void _init(
                const std::shared_ptr<IntModel>&,
                const std::shared_ptr<system::Context>&,
                const std::shared_ptr<IWidget>& parent = nullptr);

            IntSlider();

        public:
            ~IntSlider() override;

            //! Create a new integer value slider.
            static std::shared_ptr<IntSlider> create(
                const std::shared_ptr<IntModel>&,
                const std::shared_ptr<system::Context>&,
                const std::shared_ptr<IWidget>& parent = nullptr);

            //! Get the integer model.
            const std::shared_ptr<IntModel>& getModel() const;

            void setVisible(bool) override;
            void setEnabled(bool) override;
            bool acceptsKeyFocus() const override;
            void sizeHintEvent(const SizeHintEvent&) override;
            void clipEvent(bool, const ClipEvent&) override;
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

            int _posToValue(int) const;
            int _valueToPos(int) const;

            void _resetMouse();

            TLRENDER_PRIVATE();
        };
    }
}
