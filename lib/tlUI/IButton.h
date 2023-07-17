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

            //! Get whether the button is checkable.
            bool isCheckable() const;

            //! Set whether the button is checkable.
            void setCheckable(bool);

            //! Get whether the button is checked.
            bool isChecked() const;

            //! Set whether the button is checked.
            void setChecked(bool);

            //! Set the text.
            virtual void setText(const std::string&);

            //! Set the font role.
            virtual void setFontRole(FontRole);

            //! Set the icon.
            void setIcon(const std::string&);

            //! Set the checked icon.
            void setCheckedIcon(const std::string&);

            //! Set the button color role.
            void setButtonRole(ColorRole);

            //! Set the checked color role.
            void setCheckedRole(ColorRole);

            //! Set the hovered callback.
            void setHoveredCallback(const std::function<void(bool)>&);

            //! Set the pressed callback.
            void setPressedCallback(const std::function<void(void)>&);

            //! Set whether the button repeats clicks when pressed.
            void setRepeatClick(bool);

            //! Set the clicked callback.
            void setClickedCallback(const std::function<void(void)>&);

            //! Set the checked callback.
            void setCheckedCallback(const std::function<void(bool)>&);

            void setVisible(bool) override;
            void setEnabled(bool) override;
            void tickEvent(
                bool,
                bool,
                const TickEvent&) override;
            void clipEvent(
                const math::BBox2i&,
                bool,
                const ClipEvent&) override;
            void mouseEnterEvent() override;
            void mouseLeaveEvent() override;
            void mouseMoveEvent(MouseMoveEvent&) override;
            void mousePressEvent(MouseClickEvent&) override;
            void mouseReleaseEvent(MouseClickEvent&) override;

        protected:
            void _click();
            void _resetMouse();

            std::string _text;
            FontRole _fontRole = FontRole::Label;
            std::string _icon;
            std::shared_ptr<imaging::Image> _iconImage;
            std::string _checkedIcon;
            std::shared_ptr<imaging::Image> _checkedIconImage;
            ColorRole _buttonRole = ColorRole::Button;
            ColorRole _checkedRole = ColorRole::Checked;
            bool _inside = false;
            math::Vector2i _cursorPos;
            bool _pressed = false;
            bool _checked = false;
            std::function<void(bool)> _hoveredCallback;
            std::function<void(void)> _pressedCallback;
            std::function<void(void)> _clickedCallback;
            std::function<void(bool)> _checkedCallback;

        private:
            TLRENDER_PRIVATE();
        };
    }
}
