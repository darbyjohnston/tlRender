// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2025 Darby Johnston
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
                const std::shared_ptr<IntModel>&,
                const std::shared_ptr<IWidget>& parent);

            IntSlider();

        public:
            virtual ~IntSlider();

            //! Create a new widget.
            static std::shared_ptr<IntSlider> create(
                const std::shared_ptr<system::Context>&,
                const std::shared_ptr<IntModel>& = nullptr,
                const std::shared_ptr<IWidget>& parent = nullptr);

            //! Get the value.
            int getValue() const;

            //! Set the value.
            void setValue(int);

            //! Set the callback.
            void setCallback(const std::function<void(int)>&);

            //! Get the range.
            const math::IntRange& getRange() const;

            //! Set the range.
            void setRange(const math::IntRange&);

            //! Set the step.
            void setStep(int);

            //! Set the large step.
            void setLargeStep(int);

            //! Set the default value.
            void setDefaultValue(int);

            //! Get the model.
            const std::shared_ptr<IntModel>& getModel() const;

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

            int _posToValue(int) const;
            int _valueToPos(int) const;

            TLRENDER_PRIVATE();
        };
    }
}
