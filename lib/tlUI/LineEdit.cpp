// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2023 Darby Johnston
// All rights reserved.

#include <tlUI/LineEdit.h>

#include <tlUI/DrawUtil.h>
#include <tlUI/GeometryUtil.h>

namespace tl
{
    namespace ui
    {
        struct LineEdit::Private
        {
            std::string text;
            std::function<void(const std::string&)> textCallback;
            std::string format = "                    ";
            FontRole fontRole = FontRole::Mono;

            struct SizeData
            {
                int margin = 0;
                int border = 0;
                imaging::FontMetrics fontMetrics;
                math::Vector2i textSize;
                math::Vector2i formatSize;
            };
            SizeData size;

            struct DrawData
            {
                std::vector<std::shared_ptr<imaging::Glyph> > glyphs;
            };
            DrawData draw;
        };

        void LineEdit::_init(
            const std::shared_ptr<system::Context>& context,
            const std::shared_ptr<IWidget>& parent)
        {
            IWidget::_init("tl::ui::LineEdit", context, parent);
            TLRENDER_P();
            _textUpdate();
        }

        LineEdit::LineEdit() :
            _p(new Private)
        {}

        LineEdit::~LineEdit()
        {}

        std::shared_ptr<LineEdit> LineEdit::create(
            const std::shared_ptr<system::Context>& context,
            const std::shared_ptr<IWidget>& parent)
        {
            auto out = std::shared_ptr<LineEdit>(new LineEdit);
            out->_init(context, parent);
            return out;
        }

        const std::string& LineEdit::getText() const
        {
            return _p->text;
        }

        void LineEdit::setText(const std::string& value)
        {
            TLRENDER_P();
            if (value == p.text)
                return;
            p.text = value;
            _textUpdate();
        }

        void LineEdit::setTextCallback(const std::function<void(const std::string&)>& value)
        {
            _p->textCallback = value;
        }

        void LineEdit::setFormat(const std::string& value)
        {
            TLRENDER_P();
            if (value == p.format)
                return;
            p.format = value;
            _textUpdate();
        }

        void LineEdit::setFontRole(FontRole value)
        {
            TLRENDER_P();
            if (value == p.fontRole)
                return;
            p.fontRole = value;
            _updates |= Update::Size;
            _updates |= Update::Draw;
        }

        bool LineEdit::acceptsKeyFocus() const
        {
            return true;
        }

        void LineEdit::sizeHintEvent(const SizeHintEvent& event)
        {
            IWidget::sizeHintEvent(event);
            TLRENDER_P();

            p.size.margin = event.style->getSizeRole(SizeRole::MarginSmall, event.displayScale);
            p.size.border = event.style->getSizeRole(SizeRole::Border, event.displayScale);
            p.size.fontMetrics = event.getFontMetrics(p.fontRole);

            const auto fontInfo = event.style->getFontRole(p.fontRole, event.displayScale);
            p.size.textSize = event.fontSystem->measure(p.text, fontInfo);
            p.size.formatSize = event.fontSystem->measure(p.format, fontInfo);

            _sizeHint.x =
                p.size.formatSize.x +
                p.size.margin * 2 +
                p.size.border * 2;
            _sizeHint.y =
                p.size.fontMetrics.lineHeight +
                p.size.margin * 2 +
                p.size.border * 2;
        }

        void LineEdit::clipEvent(bool clipped, const ClipEvent& event)
        {
            const bool changed = clipped != _clipped;
            IWidget::clipEvent(clipped, event);
            TLRENDER_P();
            if (changed)
            {
                if (clipped)
                {
                    p.draw.glyphs.clear();
                }
                else
                {
                    const auto fontInfo = event.style->getFontRole(p.fontRole, event.displayScale);
                    p.draw.glyphs = event.fontSystem->getGlyphs(p.text, fontInfo);
                }
            }
        }

        void LineEdit::drawEvent(const DrawEvent& event)
        {
            IWidget::drawEvent(event);
            TLRENDER_P();

            const math::BBox2i g = align(
                _geometry,
                _sizeHint,
                Stretch::Expanding,
                Stretch::Expanding,
                _hAlign,
                _vAlign);

            event.render->drawMesh(
                border(g, p.size.border),
                math::Vector2i(),
                event.style->getColorRole(event.focusWidget == shared_from_this() ?
                    ColorRole::KeyFocus :
                    ColorRole::Border));

            event.render->drawRect(
                g.margin(-p.size.border),
                event.style->getColorRole(ColorRole::Base));

            const math::BBox2i g2 = g.margin(-p.size.margin);
            math::Vector2i pos(
                g2.x(),
                g2.y() + g2.h() / 2 - p.size.fontMetrics.lineHeight / 2 +
                p.size.fontMetrics.ascender);
            event.render->drawText(
                p.draw.glyphs,
                pos,
                event.style->getColorRole(ColorRole::Text));
        }

        void LineEdit::enterEvent()
        {}

        void LineEdit::leaveEvent()
        {}

        void LineEdit::mouseMoveEvent(MouseMoveEvent&)
        {}

        void LineEdit::mousePressEvent(MouseClickEvent& event)
        {
            event.accept = true;
            takeFocus();
        }

        void LineEdit::mouseReleaseEvent(MouseClickEvent&)
        {}

        void LineEdit::keyPressEvent(KeyEvent&)
        {}

        void LineEdit::keyReleaseEvent(KeyEvent&)
        {}

        void LineEdit::_textUpdate()
        {
            TLRENDER_P();
            _updates |= Update::Size;
            _updates |= Update::Draw;
        }
    }
}
