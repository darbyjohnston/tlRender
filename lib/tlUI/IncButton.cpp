// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2023 Darby Johnston
// All rights reserved.

#include <tlUI/IncButton.h>

namespace tl
{
    namespace ui
    {
        struct IncButton::Private
        {
            struct SizeData
            {
                int margin = 0;
            };
            SizeData size;
        };

        void IncButton::_init(
            const std::shared_ptr<system::Context>& context,
            const std::shared_ptr<IWidget>& parent)
        {
            IButton::_init("tl::ui::IncButton", context, parent);
            setButtonRole(ColorRole::None);
            setRepeatClick(true);
        }

        IncButton::IncButton() :
            _p(new Private)
        {}

        IncButton::~IncButton()
        {}

        std::shared_ptr<IncButton> IncButton::create(
            const std::shared_ptr<system::Context>& context,
            const std::shared_ptr<IWidget>& parent)
        {
            auto out = std::shared_ptr<IncButton>(new IncButton);
            out->_init(context, parent);
            return out;
        }

        void IncButton::sizeHintEvent(const SizeHintEvent& event)
        {
            IButton::sizeHintEvent(event);
            TLRENDER_P();

            p.size.margin = event.style->getSizeRole(SizeRole::MarginInside, event.displayScale);

            _sizeHint = math::Vector2i();
            if (_iconImage)
            {
                _sizeHint.x = _iconImage->getWidth();
                _sizeHint.y = _iconImage->getHeight();
            }
            _sizeHint.x += p.size.margin * 2;
            _sizeHint.y += p.size.margin * 2;
        }

        void IncButton::drawEvent(const DrawEvent& event)
        {
            IButton::drawEvent(event);
            TLRENDER_P();

            const math::BBox2i& g = _geometry;

            const ColorRole colorRole = _checked ?
                ColorRole::Checked :
                _buttonRole;
            if (colorRole != ColorRole::None)
            {
                event.render->drawRect(
                    g,
                    event.style->getColorRole(colorRole));
            }

            if (_pressed && _geometry.contains(_cursorPos))
            {
                event.render->drawRect(
                    g,
                    event.style->getColorRole(ColorRole::Pressed));
            }
            else if (_inside)
            {
                event.render->drawRect(
                    g,
                    event.style->getColorRole(ColorRole::Hover));
            }

            int x = g.x() + p.size.margin;
            if (_iconImage)
            {
                const auto iconSize = _iconImage->getSize();
                event.render->drawImage(
                  _iconImage,
                  math::BBox2i(
                      x,
                      g.y() + g.h() / 2 - iconSize.h / 2,
                      iconSize.w,
                      iconSize.h));
            }
        }
    }
}
