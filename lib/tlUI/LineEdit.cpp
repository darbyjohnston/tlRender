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
            size_t cursorPos = 0;
            bool cursorVisible = false;
            std::chrono::steady_clock::time_point cursorTimer;

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
                std::vector<math::BBox2i> glyphsBBox;
            };
            DrawData draw;

            struct MouseData
            {
                bool press = false;
            };
            MouseData mouse;
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
            p.cursorPos = std::min(p.cursorPos, p.text.size());
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

        void LineEdit::setVisible(bool value)
        {
            TLRENDER_P();
            const bool changed = value != _visible;
            IWidget::setVisible(value);
            if (changed && !_visible)
            {
                if (p.cursorVisible)
                {
                    p.cursorVisible = false;
                    _updates |= Update::Draw;
                }
            }
        }

        void LineEdit::setEnabled(bool value)
        {
            TLRENDER_P();
            const bool changed = value != _enabled;
            IWidget::setEnabled(value);
            if (changed && !_enabled)
            {
                if (p.cursorVisible)
                {
                    p.cursorVisible = false;
                    _updates |= Update::Draw;
                }
            }
        }

        bool LineEdit::acceptsKeyFocus() const
        {
            return true;
        }

        void LineEdit::tickEvent(const TickEvent& event)
        {
            TLRENDER_P();
            if (hasKeyFocus())
            {
                const auto now = std::chrono::steady_clock::now();
                const std::chrono::duration<float> diff = now - p.cursorTimer;
                if (diff.count() > .5F)
                {
                    p.cursorVisible = !p.cursorVisible;
                    _updates |= Update::Draw;
                    p.cursorTimer = now;
                }
            }
            else if (p.cursorVisible)
            {
                p.cursorVisible = false;
                _updates |= Update::Draw;
            }
        }

        void LineEdit::sizeHintEvent(const SizeHintEvent& event)
        {
            IWidget::sizeHintEvent(event);
            TLRENDER_P();

            p.size.margin = event.style->getSizeRole(SizeRole::MarginSmall, event.displayScale);
            p.size.border = event.style->getSizeRole(SizeRole::Border, event.displayScale);
            p.size.fontMetrics = event.getFontMetrics(p.fontRole);

            const auto fontInfo = event.style->getFontRole(p.fontRole, event.displayScale);
            p.size.textSize = event.fontSystem->getSize(p.text, fontInfo);
            p.size.formatSize = event.fontSystem->getSize(p.format, fontInfo);

            _sizeHint.x =
                p.size.formatSize.x +
                p.size.margin * 2 +
                p.size.border * 4;
            _sizeHint.y =
                p.size.fontMetrics.lineHeight +
                p.size.margin * 2 +
                p.size.border * 4;
        }

        void LineEdit::clipEvent(
            const math::BBox2i& clipRect,
            bool clipped,
            const ClipEvent& event)
        {
            IWidget::clipEvent(clipRect, clipped, event);
            TLRENDER_P();
            if (clipped)
            {
                p.draw.glyphs.clear();
                p.draw.glyphsBBox.clear();
            }
        }

        void LineEdit::drawEvent(
            const math::BBox2i& drawRect,
            const DrawEvent& event)
        {
            IWidget::drawEvent(drawRect, event);
            TLRENDER_P();

            const math::BBox2i g = _getAlignGeometry();

            if (event.focusWidget == shared_from_this())
            {
                event.render->drawMesh(
                    border(g, p.size.border * 2),
                    math::Vector2i(),
                    event.style->getColorRole(ColorRole::KeyFocus));
            }
            else
            {
                event.render->drawMesh(
                    border(g.margin(-p.size.border), p.size.border),
                    math::Vector2i(),
                    event.style->getColorRole(ColorRole::Border));
            }

            event.render->drawRect(
                g.margin(-p.size.border * 2),
                event.style->getColorRole(ColorRole::Base));

            const math::BBox2i g2 = g.margin(-(p.size.border * 2 + p.size.margin));
            math::Vector2i pos(
                g2.x(),
                g2.y() + g2.h() / 2 - p.size.fontMetrics.lineHeight / 2 +
                p.size.fontMetrics.ascender);
            if (!p.text.empty() && p.draw.glyphs.empty())
            {
                const auto fontInfo = event.style->getFontRole(p.fontRole, event.displayScale);
                p.draw.glyphs = event.fontSystem->getGlyphs(p.text, fontInfo);
                p.draw.glyphsBBox = event.fontSystem->getBBox(p.text, fontInfo);
            }
            event.render->drawText(
                p.draw.glyphs,
                pos,
                event.style->getColorRole(_enabled ?
                    ColorRole::Text :
                    ColorRole::TextDisabled));
            
            /*for (const auto& bbox : p.draw.glyphsBBox)
            {
                const math::BBox2i bbox2(
                    g2.x() + bbox.x(),
                    g2.y() + g2.h() / 2 - p.size.fontMetrics.lineHeight / 2 + bbox.y(),
                    bbox.w(),
                    bbox.h());
                event.render->drawRect(
                    bbox2,
                    imaging::Color4f(1.F, 0.F, 0.F, .2F));
            }*/

            if (p.cursorVisible)
            {
                int x = g2.x();
                if (p.cursorPos < p.draw.glyphsBBox.size())
                {
                    x += p.draw.glyphsBBox[p.cursorPos].min.x;
                }
                else if (!p.draw.glyphsBBox.empty())
                {
                    x += p.draw.glyphsBBox.back().min.x +
                        p.draw.glyphsBBox.back().w();
                }
                event.render->drawRect(
                    math::BBox2i(
                        x,
                        g2.y(),
                        p.size.border,
                        g2.h()),
                    event.style->getColorRole(ColorRole::Text));
            }
        }

        void LineEdit::enterEvent()
        {}

        void LineEdit::leaveEvent()
        {}

        void LineEdit::mouseMoveEvent(MouseMoveEvent& event)
        {
            TLRENDER_P();
            if (p.mouse.press)
            {
                event.accept = true;
                const size_t cursorPos = _getCursorPos(event.pos);
                if (cursorPos != p.cursorPos)
                {
                    p.cursorPos = cursorPos;
                    p.cursorVisible = true;
                    p.cursorTimer = std::chrono::steady_clock::now();
                    _updates |= Update::Draw;
                }
            }
        }

        void LineEdit::mousePressEvent(MouseClickEvent& event)
        {
            TLRENDER_P();
            event.accept = true;
            p.mouse.press = true;
            const size_t cursorPos = _getCursorPos(event.pos);
            if (cursorPos != p.cursorPos)
            {
                p.cursorPos = cursorPos;
                p.cursorVisible = true;
                p.cursorTimer = std::chrono::steady_clock::now();
                _updates |= Update::Draw;
            }
            takeFocus();
        }

        void LineEdit::mouseReleaseEvent(MouseClickEvent& event)
        {
            TLRENDER_P();
            event.accept = true;
            p.mouse.press = false;
        }

        void LineEdit::keyPressEvent(KeyEvent& event)
        {
            TLRENDER_P();
            switch (event.key)
            {
            case Key::Left:
                if (p.cursorPos > 0)
                {
                    event.accept = true;
                    p.cursorPos--;
                    p.cursorVisible = true;
                    p.cursorTimer = std::chrono::steady_clock::now();
                    _updates |= Update::Draw;
                }
                break;
            case Key::Right:
                if (p.cursorPos < p.text.size())
                {
                    event.accept = true;
                    p.cursorPos++;
                    p.cursorVisible = true;
                    p.cursorTimer = std::chrono::steady_clock::now();
                    _updates |= Update::Draw;
                }
                break;
            case Key::Home:
                if (p.cursorPos > 0)
                {
                    event.accept = true;
                    p.cursorPos = 0;
                    p.cursorVisible = true;
                    p.cursorTimer = std::chrono::steady_clock::now();
                    _updates |= Update::Draw;
                }
                break;
            case Key::End:
                if (p.cursorPos < p.text.size())
                {
                    event.accept = true;
                    p.cursorPos = p.text.size();
                    p.cursorVisible = true;
                    p.cursorTimer = std::chrono::steady_clock::now();
                    _updates |= Update::Draw;
                }
                break;
            case Key::Backspace:
                if (p.cursorPos > 0)
                {
                    event.accept = true;
                    const auto i = p.text.begin() + p.cursorPos - 1;
                    p.text.erase(i);
                    p.cursorPos--;
                    _textUpdate();
                }
                break;
            case Key::Delete:
                if (p.cursorPos < p.text.size())
                {
                    event.accept = true;
                    const auto i = p.text.begin() + p.cursorPos;
                    p.text.erase(i);
                    _textUpdate();
                }
                break;
            case Key::Escape:
                if (hasKeyFocus())
                {
                    event.accept = true;
                    releaseFocus();
                }
                break;
            }
        }

        void LineEdit::keyReleaseEvent(KeyEvent& event)
        {
            event.accept = true;
        }

        void LineEdit::textEvent(TextEvent& event)
        {
            TLRENDER_P();
            event.accept = true;
            p.text.insert(p.cursorPos, event.text);
            p.cursorPos += event.text.size();
            _textUpdate();
        }

        math::BBox2i LineEdit::_getAlignGeometry() const
        {
            return align(
                _geometry,
                _sizeHint,
                Stretch::Expanding,
                Stretch::Expanding,
                _hAlign,
                _vAlign);
        }

        size_t LineEdit::_getCursorPos(const math::Vector2i& value)
        {
            TLRENDER_P();
            size_t out = 0;
            const math::BBox2i g = _getAlignGeometry();
            const math::BBox2i g2 = g.margin(-p.size.border * 2);
            const math::Vector2i pos(
                math::clamp(value.x, g2.min.x, g2.max.x - 1),
                math::clamp(value.y, g2.min.y, g2.max.y - 1));
            math::BBox2i bbox(
                g2.x(),
                g2.y(),
                0,
                g2.h());
            for (const auto& glyphBBox : p.draw.glyphsBBox)
            {
                bbox.max.x = g2.x() + glyphBBox.x() + glyphBBox.w();
                if (bbox.contains(pos))
                {
                    break;
                }
                ++out;
            }
            return out;
        }

        void LineEdit::_textUpdate()
        {
            TLRENDER_P();
            p.draw.glyphs.clear();
            p.draw.glyphsBBox.clear();
            _updates |= Update::Draw;
        }
    }
}
