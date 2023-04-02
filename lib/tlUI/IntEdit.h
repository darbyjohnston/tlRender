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
        //! Integer number editor.
        class IntEdit : public IWidget
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

            //! Set the font information.
            void setFontInfo(const imaging::FontInfo&);

            void sizeEvent(const SizeEvent&) override;
            void drawEvent(const DrawEvent&) override;

        private:
            void _textUpdate();

            TLRENDER_PRIVATE();
        };
    }
}
