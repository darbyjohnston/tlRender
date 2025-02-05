// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2025 Darby Johnston
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
            DTK_NON_COPYABLE(DoubleSlider);

        protected:
            void _init(
                const std::shared_ptr<dtk::Context>&,
                const std::shared_ptr<DoubleModel>&,
                const std::shared_ptr<IWidget>& parent);

            DoubleSlider();

        public:
            virtual ~DoubleSlider();

            //! Create a new widget.
            static std::shared_ptr<DoubleSlider> create(
                const std::shared_ptr<dtk::Context>&,
                const std::shared_ptr<DoubleModel>& = nullptr,
                const std::shared_ptr<IWidget>& parent = nullptr);

            //! Get the value.
            double getValue() const;

            //! Set the value.
            void setValue(double);

            //! Set the callback.
            void setCallback(const std::function<void(double)>&);

            //! Get the range.
            const dtk::RangeD& getRange() const;

            //! Set the range.
            void setRange(const dtk::RangeD&);

            //! Set the step.
            void setStep(double);

            //! Set the large step.
            void setLargeStep(double);

            //! Set the default value.
            void setDefaultValue(double);

            //! Get the model.
            const std::shared_ptr<DoubleModel>& getModel() const;

            void sizeHintEvent(const SizeHintEvent&) override;
            void drawEvent(const dtk::Box2I&, const DrawEvent&) override;
            void mouseEnterEvent() override;
            void mouseLeaveEvent() override;
            void mouseMoveEvent(MouseMoveEvent&) override;
            void mousePressEvent(MouseClickEvent&) override;
            void mouseReleaseEvent(MouseClickEvent&) override;
            void keyPressEvent(KeyEvent&) override;
            void keyReleaseEvent(KeyEvent&) override;

        private:
            dtk::Box2I _getSliderGeometry() const;

            double _posToValue(int) const;
            int _valueToPos(double) const;

            DTK_PRIVATE();
        };
    }
}
