// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2023 Darby Johnston
// All rights reserved.

#pragma once

#include <tlUI/IWidget.h>

namespace tl
{
    namespace ui
    {
        //! Base class for buttons.
        class IButton : public IWidget
        {
            TLRENDER_NON_COPYABLE(IButton);

        protected:
            void _init(
                const std::string& name,
                const std::shared_ptr<system::Context>&,
                const std::shared_ptr<IWidget>& parent = nullptr);

            IButton();

        public:
            ~IButton() override;

            //! Set whether the button is checkable.
            void setCheckable(bool);

            //! Set whether the button is checked.
            void setChecked(bool);

            //! Set the text.
            void setText(const std::string&);

            //! Set the font role.
            void setFontRole(FontRole);

            //! Set the icon.
            void setIcon(const std::string&);

            //! Set the button role.
            void setButtonRole(ColorRole);
            
            //! Set the clicked callback.
            void setClickedCallback(const std::function<void(void)>&);

            //! Set the checked callback.
            void setCheckedCallback(const std::function<void(bool)>&);

            void tickEvent(const TickEvent&) override;
            void enterEvent() override;
            void leaveEvent() override;
            void mouseMoveEvent(MouseMoveEvent&) override;
            void mousePressEvent(MouseClickEvent&) override;
            void mouseReleaseEvent(MouseClickEvent&) override;

        protected:
            void _click();

            std::string _text;
            FontRole _fontRole = FontRole::Label;
            std::shared_ptr<imaging::Image> _iconImage;
            ColorRole _buttonRole = ColorRole::Button;
            bool _inside = false;
            math::Vector2i _cursorPos;
            bool _pressed = false;
            bool _checked = false;
            std::function<void(void)> _clickedCallback;
            std::function<void(bool)> _checkedCallback;

        private:
            TLRENDER_PRIVATE();
        };
    }
}
