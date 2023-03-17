// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2023 Darby Johnston
// All rights reserved.

#pragma once

#include <tlUI/IWidget.h>

#include <tlCore/ValueObserver.h>

namespace tl
{
    namespace ui
    {
        //! Text button.
        class TextButton : public IWidget
        {
            TLRENDER_NON_COPYABLE(TextButton);

        protected:
            void _init(
                const std::shared_ptr<system::Context>&,
                const std::shared_ptr<IWidget>& parent = nullptr);

            TextButton();

        public:
            ~TextButton() override;

            //! Create a new text button.
            static std::shared_ptr<TextButton> create(
                const std::shared_ptr<system::Context>&,
                const std::shared_ptr<IWidget>& parent = nullptr);

            //! Get the text.
            const std::string& getText() const;

            //! Set the text.
            void setText(const std::string&);

            //! Get the font information.
            const imaging::FontInfo& getFontInfo() const;

            //! Set the font information.
            void setFontInfo(const imaging::FontInfo&);

            //! Observe the clicks.
            std::shared_ptr<observer::IValue<bool> > observeClick() const;

            void sizeHintEvent(const SizeHintEvent&) override;
            void drawEvent(const DrawEvent&) override;
            void enterEvent() override;
            void leaveEvent() override;
            void mouseMoveEvent(const MouseMoveEvent&) override;
            void mousePressEvent(const MouseClickEvent&) override;
            void mouseReleaseEvent(const MouseClickEvent&) override;

        private:
            TLRENDER_PRIVATE();
        };
    }
}
