// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2023 Darby Johnston
// All rights reserved.

#include <tlUI/IButton.h>

#include <tlUI/DrawUtil.h>

namespace tl
{
    namespace ui
    {
        struct IButton::Private
        {
            bool checkable = false;
            std::string icon;
            bool iconInit = false;
            float iconContentScale = 1.F;
            std::future<std::shared_ptr<imaging::Image> > iconFuture;
            std::shared_ptr<observer::Value<bool> > click;
        };

        void IButton::_init(
            const std::string& name,
            const std::shared_ptr<system::Context>& context,
            const std::shared_ptr<IWidget>& parent)
        {
            IWidget::_init(name, context, parent);
            TLRENDER_P();
            p.click = observer::Value<bool>::create(false);
            _checked = observer::Value<bool>::create(false);
        }

        IButton::IButton() :
            _p(new Private)
        {}

        IButton::~IButton()
        {}

        void IButton::setCheckable(bool value)
        {
            TLRENDER_P();
            p.checkable = value;
            if (!p.checkable)
            {
                if (_checked->setIfChanged(false))
                {
                    _updates |= Update::Draw;
                }
            }
        }

        void IButton::setChecked(bool value)
        {
            if (_checked->setIfChanged(value))
            {
                _updates |= Update::Draw;
            }
        }

        void IButton::setText(const std::string& value)
        {
            _text = value;
        }

        void IButton::setFontInfo(const imaging::FontInfo& value)
        {
            _fontInfo = value;
        }
        
        void IButton::setIcon(const std::string& icon)
        {
            _p->icon = icon;
            _p->iconInit = true;
            _iconImage.reset();
        }

        void IButton::setButtonRole(ColorRole value)
        {
            if (value == _buttonRole)
                return;
            _buttonRole = value;
            _updates |= Update::Draw;
        }

        std::shared_ptr<observer::IValue<bool> > IButton::observeClick() const
        {
            return _p->click;
        }

        std::shared_ptr<observer::IValue<bool> > IButton::observeChecked() const
        {
            return _checked;
        }

        void IButton::tickEvent(const TickEvent& event)
        {
            TLRENDER_P();
            if (event.contentScale != p.iconContentScale)
            {
                p.iconInit = true;
                p.iconFuture = std::future<std::shared_ptr<imaging::Image> >();
                _iconImage.reset();
            }
            if (!p.icon.empty() && p.iconInit)
            {
                p.iconInit = false;
                p.iconContentScale = event.contentScale;
                p.iconFuture = event.iconLibrary->request(p.icon, event.contentScale);
            }
            if (p.iconFuture.valid() &&
                p.iconFuture.wait_for(std::chrono::seconds(0)) == std::future_status::ready)
            {
                _iconImage = p.iconFuture.get();
                _updates |= Update::Size;
                _updates |= Update::Draw;
            }
        }

        void IButton::enterEvent()
        {
            _inside = true;
            _updates |= Update::Draw;
        }

        void IButton::leaveEvent()
        {
            _inside = false;
            _updates |= Update::Draw;
        }

        void IButton::mouseMoveEvent(const MouseMoveEvent& event)
        {
            _cursorPos = event.pos;
        }

        void IButton::mousePressEvent(const MouseClickEvent&)
        {
            _pressed = true;
            _updates |= Update::Draw;
        }

        void IButton::mouseReleaseEvent(const MouseClickEvent&)
        {
            TLRENDER_P();
            _pressed = false;
            if (_geometry.contains(_cursorPos))
            {
                p.click->setAlways(true);
                if (p.checkable)
                {
                    _checked->setIfChanged(!_checked->get());
                }
            }
            _updates |= Update::Draw;
        }
    }
}
