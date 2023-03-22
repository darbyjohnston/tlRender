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

            //! Set the font information.
            void setFontInfo(const imaging::FontInfo&);

            //! Set the icon.
            void setIcon(const std::string&);

            //! Set the button role.
            void setButtonRole(ColorRole);
            
            //! Observe button clicks.
            std::shared_ptr<observer::IValue<bool> > observeClick() const;

            //! Observe the checked state.
            std::shared_ptr<observer::IValue<bool> > observeChecked() const;

            void tickEvent(const TickEvent&) override;
            void enterEvent() override;
            void leaveEvent() override;
            void mouseMoveEvent(const MouseMoveEvent&) override;
            void mousePressEvent(const MouseClickEvent&) override;
            void mouseReleaseEvent(const MouseClickEvent&) override;

        protected:
            std::string _text;
            imaging::FontInfo _fontInfo;
            std::shared_ptr<imaging::Image> _iconImage;
            ColorRole _buttonRole = ColorRole::Button;
            bool _inside = false;
            math::Vector2i _cursorPos;
            bool _pressed = false;
            std::shared_ptr<observer::Value<bool> > _checked;

        private:
            TLRENDER_PRIVATE();
        };
    }
}
