// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2023 Darby Johnston
// All rights reserved.

#pragma once

#include <tlUI/LineEdit.h>
#include <tlUI/IntModel.h>

namespace tl
{
    namespace ui
    {
        //! Integer number editor.
        class IntEdit : public LineEdit
        {
            TLRENDER_NON_COPYABLE(IntEdit);

        protected:
            void _init(
                const std::shared_ptr<system::Context>&,
                const std::shared_ptr<IWidget>& parent = nullptr);

            IntEdit();

        public:
            ~IntEdit() override;

            //! Create a new integer number editor.
            static std::shared_ptr<IntEdit> create(
                const std::shared_ptr<system::Context>&,
                const std::shared_ptr<IWidget>& parent = nullptr);

            //! Get the integer model.
            const std::shared_ptr<IntModel>& getModel() const;

            //! Set the integer model.
            void setModel(const std::shared_ptr<IntModel>&);

            //! Set the number of digits to display.
            void setDigits(int);

            void keyPressEvent(KeyEvent&) override;
            void keyReleaseEvent(KeyEvent&) override;

        private:
            void _intUpdate();

            TLRENDER_PRIVATE();
        };
    }
}
