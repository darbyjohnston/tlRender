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
            virtual ~FloatSlider();

            //! Create a new widget.
            static std::shared_ptr<FloatSlider> create(
                const std::shared_ptr<system::Context>&,
                const std::shared_ptr<FloatModel>& = nullptr,
                const std::shared_ptr<IWidget>& parent = nullptr);

            //! Get the value.
            float getValue() const;

            //! Set the value.
            void setValue(float);

            //! Set the callback.
            void setCallback(const std::function<void(float)>&);

            //! Get the range.
            const math::FloatRange& getRange() const;

            //! Set the range.
            void setRange(const math::FloatRange&);

            //! Set the step.
            void setStep(float);

            //! Set the large step.
            void setLargeStep(float);

            //! Set the default value.
            void setDefaultValue(float);

            //! Get the model.
            const std::shared_ptr<FloatModel>& getModel() const;

            void sizeHintEvent(const SizeHintEvent&) override;
            void drawEvent(const math::Box2i&, const DrawEvent&) override;
            void mouseEnterEvent() override;
            void mouseLeaveEvent() override;
            void mouseMoveEvent(MouseMoveEvent&) override;
            void mousePressEvent(MouseClickEvent&) override;
            void mouseReleaseEvent(MouseClickEvent&) override;
            void keyPressEvent(KeyEvent&) override;
            void keyReleaseEvent(KeyEvent&) override;

        private:
            math::Box2i _getSliderGeometry() const;

            float _posToValue(int) const;
            int _valueToPos(float) const;

            TLRENDER_PRIVATE();
        };
    }
}
