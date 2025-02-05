// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2025 Darby Johnston
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
            float iconScale = 1.F;
            bool iconInit = false;
            bool checkedIconInit = false;
            bool repeatClick = false;
            bool repeatClickInit = false;
            std::chrono::steady_clock::time_point repeatClickTimer;
        };

        void IButton::_init(
            const std::string& objectName,
            const std::shared_ptr<dtk::Context>& context,
            const std::shared_ptr<IWidget>& parent)
        {
            IWidget::_init(objectName, context, parent);
            _setMouseHover(true);
            _setMousePress(true);
        }

        IButton::IButton() :
            _p(new Private)
        {}

        IButton::~IButton()
        {}

        bool IButton::isCheckable() const
        {
            return _p->checkable;
        }

        void IButton::setCheckable(bool value)
        {
            DTK_P();
            if (value == p.checkable)
                return;
            p.checkable = value;
            if (!p.checkable && _checked)
            {
                _checked = false;
                _updates |= Update::Draw;
            }
        }

        bool IButton::isChecked() const
        {
            return _checked;
        }

        void IButton::setChecked(bool value)
        {
            DTK_P();
            if (value == _checked)
                return;
            _checked = value;
            _updates |= Update::Draw;
        }

        void IButton::setText(const std::string& value)
        {
            if (value == _text)
                return;
            _text = value;
            _updates |= Update::Size;
            _updates |= Update::Draw;
        }

        void IButton::setFontRole(FontRole value)
        {
            if (value == _fontRole)
                return;
            _fontRole = value;
            _updates |= Update::Size;
            _updates |= Update::Draw;
        }

        void IButton::setIcon(const std::string& icon)
        {
            DTK_P();
            _icon = icon;
            p.iconInit = true;
            _iconImage.reset();
        }

        void IButton::setCheckedIcon(const std::string& icon)
        {
            DTK_P();
            _checkedIcon = icon;
            p.checkedIconInit = true;
            _checkedIconImage.reset();
        }

        void IButton::setButtonRole(ColorRole value)
        {
            if (value == _buttonRole)
                return;
            _buttonRole = value;
            _updates |= Update::Draw;
        }

        void IButton::setCheckedRole(ColorRole value)
        {
            if (value == _checkedRole)
                return;
            _checkedRole = value;
            _updates |= Update::Draw;
        }

        void IButton::setRepeatClick(bool value)
        {
            DTK_P();
            p.repeatClick = value;
        }

        void IButton::setHoveredCallback(const std::function<void(bool)>& value)
        {
            _hoveredCallback = value;
        }

        void IButton::setPressedCallback(const std::function<void(void)>& value)
        {
            _pressedCallback = value;
        }

        void IButton::setClickedCallback(const std::function<void(void)>& value)
        {
            _clickedCallback = value;
        }

        void IButton::setCheckedCallback(const std::function<void(bool)>& value)
        {
            _checkedCallback = value;
        }

        void IButton::tickEvent(
            bool parentsVisible,
            bool parentsEnabled,
            const TickEvent& event)
        {
            IWidget::tickEvent(parentsVisible, parentsEnabled, event);
            DTK_P();
            if (_mouse.press && p.repeatClick)
            {
                const float duration = p.repeatClickInit ? .4F : .02F;
                const auto now = std::chrono::steady_clock::now();
                const std::chrono::duration<float> diff = now - p.repeatClickTimer;
                if (diff.count() > duration)
                {
                    _click();
                    p.repeatClickInit = false;
                    p.repeatClickTimer = now;
                }
            }
        }

        void IButton::sizeHintEvent(const SizeHintEvent& event)
        {
            IWidget::sizeHintEvent(event);
            DTK_P();
            if (_displayScale != p.iconScale)
            {
                p.iconScale = _displayScale;
                p.iconInit = true;
                _iconImage.reset();
                p.checkedIconInit = true;
                _checkedIconImage.reset();
            }
            if (!_icon.empty() && p.iconInit)
            {
                p.iconInit = false;
                _iconImage = event.iconLibrary->request(_icon, _displayScale).get();
            }
            if (!_checkedIcon.empty() && p.checkedIconInit)
            {
                p.checkedIconInit = false;
                _checkedIconImage = event.iconLibrary->request(_checkedIcon, _displayScale).get();
            }
        }

        void IButton::mouseEnterEvent()
        {
            IWidget::mouseEnterEvent();
            _updates |= Update::Draw;
            if (_hoveredCallback)
            {
                _hoveredCallback(_mouse.inside);
            }
        }

        void IButton::mouseLeaveEvent()
        {
            IWidget::mouseLeaveEvent();
            _updates |= Update::Draw;
            if (_hoveredCallback)
            {
                _hoveredCallback(_mouse.inside);
            }
        }

        void IButton::mousePressEvent(MouseClickEvent& event)
        {
            IWidget::mousePressEvent(event);
            DTK_P();
            if (acceptsKeyFocus())
            {
                takeKeyFocus();
            }
            _updates |= Update::Draw;
            if (_pressedCallback)
            {
                _pressedCallback();
            }
            if (p.repeatClick)
            {
                p.repeatClickInit = true;
                p.repeatClickTimer = std::chrono::steady_clock::now();
            }
        }

        void IButton::mouseReleaseEvent(MouseClickEvent& event)
        {
            IWidget::mouseReleaseEvent(event);
            _updates |= Update::Draw;
            if (dtk::contains(_geometry, _mouse.pos))
            {
                _click();
            }
        }

        void IButton::_click()
        {
            DTK_P();
            if (_clickedCallback)
            {
                _clickedCallback();
            }
            if (p.checkable)
            {
                _checked = !_checked;
                _updates |= Update::Draw;
                if (_checkedCallback)
                {
                    _checkedCallback(_checked);
                }
            }
        }

        void IButton::_releaseMouse()
        {
            const bool inside = _mouse.inside;
            IWidget::_releaseMouse();
            if (inside)
            {
                if (_hoveredCallback)
                {
                    _hoveredCallback(false);
                }
            }
        }
    }
}
