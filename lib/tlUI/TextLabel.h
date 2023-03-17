// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2023 Darby Johnston
// All rights reserved.

#pragma once

#include <tlUI/IWidget.h>

namespace tl
{
    namespace ui
    {
        //! Text label.
        class TextLabel : public IWidget
        {
            TLRENDER_NON_COPYABLE(TextLabel);

        protected:
            void _init(
                const std::shared_ptr<system::Context>&,
                const std::shared_ptr<IWidget>& parent = nullptr);

            TextLabel();

        public:
            ~TextLabel() override;

            //! Create a new text label.
            static std::shared_ptr<TextLabel> create(
                const std::shared_ptr<system::Context>&,
                const std::shared_ptr<IWidget>& parent = nullptr);

            //! Get the text.
            const std::string& getText() const;

            //! Set the text.
            void setText(const std::string&);

            void sizeHint(const SizeHintData&) override;
            void draw(const DrawData&) override;

        private:
            TLRENDER_PRIVATE();
        };
    }
}
