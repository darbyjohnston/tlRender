// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2023 Darby Johnston
// All rights reserved.

#include <tlUI/PushButton.h>

#include <tlUI/DrawUtil.h>

namespace tl
{
    namespace ui
    {
        struct PushButton::Private
        {
            bool checkable = false;
            std::string text;
            imaging::FontInfo fontInfo;
            bool border = true;
            bool inside = false;
            math::Vector2i cursorPos;
            bool pressed = false;
            std::shared_ptr<observer::Value<bool> > click;
            std::shared_ptr<observer::Value<bool> > checked;
        };

        void PushButton::_init(
            const std::shared_ptr<system::Context>& context,
            const std::shared_ptr<IWidget>& parent)
        {
            IWidget::_init("tl::ui::PushButton", context, parent);
            TLRENDER_P();
            setStretch(Stretch::Expanding, Orientation::Horizontal);
            setBackgroundRole(ColorRole::Button);
            p.click = observer::Value<bool>::create(false);
            p.checked = observer::Value<bool>::create(false);
        }

        PushButton::PushButton() :
            _p(new Private)
        {}

        PushButton::~PushButton()
        {}

        std::shared_ptr<PushButton> PushButton::create(
            const std::shared_ptr<system::Context>& context,
            const std::shared_ptr<IWidget>& parent)
        {
            auto out = std::shared_ptr<PushButton>(new PushButton);
            out->_init(context, parent);
            return out;
        }

        void PushButton::setCheckable(bool value)
        {
            _p->checkable = value;
            if (!_p->checkable)
            {
                _p->checked->setIfChanged(false);
            }
        }

        void PushButton::setChecked(bool value)
        {
            _p->checked->setIfChanged(value);
        }

        void PushButton::setText(const std::string& value)
        {
            _p->text = value;
        }

        void PushButton::setFontInfo(const imaging::FontInfo& value)
        {
            _p->fontInfo = value;
        }

        void PushButton::setBorder(bool value)
        {
            _p->border = value;
        }

        std::shared_ptr<observer::IValue<bool> > PushButton::observeClick() const
        {
            return _p->click;
        }

        std::shared_ptr<observer::IValue<bool> > PushButton::observeChecked() const
        {
            return _p->checked;
        }

        void PushButton::sizeHintEvent(const SizeHintEvent& event)
        {
            TLRENDER_P();
            const int m = event.style->getSizeRole(SizeRole::Margin) * event.contentScale;
            imaging::FontInfo fontInfo = p.fontInfo;
            fontInfo.size *= event.contentScale;
            auto fontMetrics = event.fontSystem->getMetrics(fontInfo);
            _sizeHint.x = event.fontSystem->measure(p.text, fontInfo).x + m * 2;
            _sizeHint.y = fontMetrics.lineHeight + m * 2;
        }

        void PushButton::drawEvent(const DrawEvent& event)
        {
            IWidget::drawEvent(event);
            TLRENDER_P();

            const int m = event.style->getSizeRole(SizeRole::Margin) * event.contentScale;
            const int b = event.style->getSizeRole(SizeRole::Border) * event.contentScale;
            math::BBox2i g = _geometry;

            if (p.border)
            {
                event.render->drawMesh(
                    border(g, b),
                    lighter(event.style->getColorRole(ColorRole::Button), .1F));
                g = g.margin(-b);
            }

            if (p.checked->get())
            {
                event.render->drawRect(
                    g,
                    event.style->getColorRole(ColorRole::Checked));
            }

            if (p.pressed && _geometry.contains(p.cursorPos))
            {
                event.render->drawRect(
                    g,
                    event.style->getColorRole(ColorRole::Pressed));
            }
            else if (p.inside)
            {
                event.render->drawRect(
                    g,
                    event.style->getColorRole(ColorRole::Hover));
            }

            imaging::FontInfo fontInfo = p.fontInfo;
            fontInfo.size *= event.contentScale;
            auto fontMetrics = event.fontSystem->getMetrics(fontInfo);
            math::Vector2i textSize = event.fontSystem->measure(p.text, fontInfo);
            event.render->drawText(
                event.fontSystem->getGlyphs(p.text, fontInfo),
                math::Vector2i(
                    g.x() + g.w() / 2 - textSize.x / 2,
                    g.y() + g.h() / 2 - textSize.y / 2 + fontMetrics.ascender),
                event.style->getColorRole(ColorRole::Text));
        }

        void PushButton::enterEvent()
        {
            _p->inside = true;
        }

        void PushButton::leaveEvent()
        {
            _p->inside = false;
        }

        void PushButton::mouseMoveEvent(const MouseMoveEvent& event)
        {
            _p->cursorPos = event.pos;
        }

        void PushButton::mousePressEvent(const MouseClickEvent&)
        {
            _p->pressed = true;
        }

        void PushButton::mouseReleaseEvent(const MouseClickEvent&)
        {
            TLRENDER_P();
            p.pressed = false;
            if (_geometry.contains(p.cursorPos))
            {
                p.click->setAlways(true);
                if (p.checkable)
                {
                    p.checked->setIfChanged(!p.checked->get());
                }
            }
        }
    }
}
