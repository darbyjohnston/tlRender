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
        //! Integer value slider.
        class IntSlider : public IWidget
        {
            TLRENDER_NON_COPYABLE(IntSlider);

        protected:
            void _init(
                const std::shared_ptr<system::Context>&,
                const std::shared_ptr<IWidget>& parent = nullptr);

            IntSlider();

        public:
            ~IntSlider() override;

            //! Create a new integer value slider.
            static std::shared_ptr<IntSlider> create(
                const std::shared_ptr<system::Context>&,
                const std::shared_ptr<IWidget>& parent = nullptr);

            //! Get the integer model.
            const std::shared_ptr<IntModel>& getModel() const;

            //! Set the integer model.
            void setModel(const std::shared_ptr<IntModel>&);

            void sizeEvent(const SizeEvent&) override;
            void drawEvent(const DrawEvent&) override;
            void enterEvent() override;
            void leaveEvent() override;
            void mouseMoveEvent(MouseMoveEvent&) override;
            void mousePressEvent(MouseClickEvent&) override;
            void mouseReleaseEvent(MouseClickEvent&) override;
            void keyPressEvent(KeyEvent&) override;

        private:
            math::BBox2i _getSliderGeometry() const;

            int _posToValue(int) const;
            int _valueToPos(int) const;

            TLRENDER_PRIVATE();
        };
    }
}
