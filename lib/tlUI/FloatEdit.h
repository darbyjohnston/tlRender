// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2023 Darby Johnston
// All rights reserved.

#pragma once

#include <tlUI/LineEdit.h>
#include <tlUI/FloatModel.h>

namespace tl
{
    namespace ui
    {
        //! Floating point number editor.
        class FloatEdit : public LineEdit
        {
            TLRENDER_NON_COPYABLE(FloatEdit);

        protected:
            void _init(
                const std::shared_ptr<system::Context>&,
                const std::shared_ptr<IWidget>& parent = nullptr);

            FloatEdit();

        public:
            ~FloatEdit() override;

            //! Create a new floating point number editor.
            static std::shared_ptr<FloatEdit> create(
                const std::shared_ptr<system::Context>&,
                const std::shared_ptr<IWidget>& parent = nullptr);

            //! Get the floating point model.
            const std::shared_ptr<FloatModel>& getModel() const;

            //! Set the floating point model.
            void setModel(const std::shared_ptr<FloatModel>&);

            //! Set the number of digits to display.
            void setDigits(int);

            //! Set the display precision.
            void setPrecision(int);

            void keyPressEvent(KeyEvent&) override;
            void keyReleaseEvent(KeyEvent&) override;

        private:
            void _floatUpdate();

            TLRENDER_PRIVATE();
        };
    }
}
