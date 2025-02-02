// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2025 Darby Johnston
// All rights reserved.

#include <tlUI/LineEdit.h>

#include <tlUI/DrawUtil.h>
#include <tlUI/IClipboard.h>
#include <tlUI/IWindow.h>
#include <tlUI/LayoutUtil.h>

#include <tlTimeline/RenderUtil.h>

namespace tl
{
    namespace ui
    {
        namespace
        {
            typedef std::pair<int, int> SelectionPair;

            class Selection
            {
            public:
                const SelectionPair& get() const;
                SelectionPair getSorted() const;
                bool isValid() const;
                void set(const SelectionPair&);
                void setFirst(int);
                void setSecond(int);

                void select(int first, int second);
                void clear();

            private:
                SelectionPair _pair = std::make_pair(-1, -1);
            };

            const SelectionPair& Selection::get() const
            {
                return _pair;
            }

            SelectionPair Selection::getSorted() const
            {
                return std::make_pair(
                    std::min(_pair.first, _pair.second),
                    std::max(_pair.first, _pair.second));
            }

            bool Selection::isValid() const
            {
                return
                    _pair.first != -1 &&
                    _pair.second != -1 &&
                    _pair.first != _pair.second;
            }

            void Selection::set(const SelectionPair& value)
            {
                _pair = value;
            }

            void Selection::setFirst(int value)
            {
                _pair.first = value;
            }

            void Selection::setSecond(int value)
            {
                _pair.second = value;
            }

            void Selection::select(int first, int second)
            {
                if (-1 == _pair.first)
                {
                    _pair.first = first;
                    _pair.second = second;
                }
                else
                {
                    _pair.second = second;
                }
            }

            void Selection::clear()
            {
                _pair = std::make_pair(-1, -1);
            }
        }

        struct LineEdit::Private
        {
            std::string text;
            std::function<void(const std::string&)> textCallback;
            std::function<void(const std::string&)> textChangedCallback;
            std::string format = "                    ";
            std::function<void(bool)> focusCallback;
            FontRole fontRole = FontRole::Mono;
            int cursorPos = 0;
            bool cursorVisible = false;
            std::chrono::steady_clock::time_point cursorTimer;
            Selection selection;

            struct SizeData
            {
                bool sizeInit = true;
                int margin = 0;
                int border = 0;

                bool textInit = true;
                image::FontInfo fontInfo;
                image::FontMetrics fontMetrics;
                math::Size2i textSize;
                math::Size2i formatSize;
            };
            SizeData size;

            struct DrawData
            {
                std::vector<std::shared_ptr<image::Glyph> > glyphs;
                std::vector<math::Box2i> glyphsBox;
            };
            DrawData draw;
        };

        void LineEdit::_init(
            const std::shared_ptr<dtk::Context>& context,
            const std::shared_ptr<IWidget>& parent)
        {
            IWidget::_init("tl::ui::LineEdit", context, parent);
            TLRENDER_P();
            setAcceptsKeyFocus(true);
            _setMouseHover(true);
            _setMousePress(true);
            _textUpdate();
        }

        LineEdit::LineEdit() :
            _p(new Private)
        {}

        LineEdit::~LineEdit()
        {}

        std::shared_ptr<LineEdit> LineEdit::create(
            const std::shared_ptr<dtk::Context>& context,
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
            p.cursorPos = p.text.size();
            _textUpdate();
        }

        void LineEdit::clearText()
        {
            setText(std::string());
        }

        void LineEdit::setTextCallback(const std::function<void(const std::string&)>& value)
        {
            _p->textCallback = value;
        }

        void LineEdit::setTextChangedCallback(const std::function<void(const std::string&)>& value)
        {
            _p->textChangedCallback = value;
        }

        void LineEdit::setFormat(const std::string& value)
        {
            TLRENDER_P();
            if (value == p.format)
                return;
            p.format = value;
            _textUpdate();
        }

        void LineEdit::setFocusCallback(const std::function<void(bool)>& value)
        {
            _p->focusCallback = value;
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

        void LineEdit::tickEvent(
            bool parentsVisible,
            bool parentsEnabled,
            const TickEvent& event)
        {
            IWidget::tickEvent(parentsVisible, parentsEnabled, event);
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
            const bool displayScaleChanged = event.displayScale != _displayScale;
            IWidget::sizeHintEvent(event);
            TLRENDER_P();

            if (displayScaleChanged || p.size.sizeInit)
            {
                p.size.margin = event.style->getSizeRole(SizeRole::MarginInside, _displayScale);
                p.size.border = event.style->getSizeRole(SizeRole::Border, _displayScale);
            }
            if (displayScaleChanged || p.size.textInit || p.size.sizeInit)
            {
                p.size.fontInfo = event.style->getFontRole(p.fontRole, _displayScale);
                p.size.fontMetrics = event.fontSystem->getMetrics(p.size.fontInfo);
                p.size.textSize = event.fontSystem->getSize(p.text, p.size.fontInfo);
                p.size.formatSize = event.fontSystem->getSize(p.format, p.size.fontInfo);
                p.draw.glyphs.clear();
                p.draw.glyphsBox.clear();
            }
            p.size.sizeInit = false;
            p.size.textInit = false;

            _sizeHint.w =
                p.size.formatSize.w +
                p.size.margin * 2 +
                p.size.border * 4;
            _sizeHint.h =
                p.size.fontMetrics.lineHeight +
                p.size.margin * 2 +
                p.size.border * 4;
        }

        void LineEdit::clipEvent(const math::Box2i& clipRect, bool clipped)
        {
            IWidget::clipEvent(clipRect, clipped);
            TLRENDER_P();
            if (clipped)
            {
                p.draw.glyphs.clear();
                p.draw.glyphsBox.clear();
            }
        }

        void LineEdit::drawEvent(
            const math::Box2i& drawRect,
            const DrawEvent& event)
        {
            IWidget::drawEvent(drawRect, event);
            TLRENDER_P();

            const math::Box2i g = _getAlignGeometry();
            const bool enabled = isEnabled();

            if (_keyFocus)
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

            const timeline::ClipRectEnabledState clipRectEnabledState(event.render);
            const timeline::ClipRectState clipRectState(event.render);
            event.render->setClipRectEnabled(true);
            event.render->setClipRect(g.margin(-p.size.border * 2).intersect(drawRect));

            const math::Box2i g2 = g.margin(-(p.size.border * 2 + p.size.margin));
            if (p.selection.isValid())
            {
                const auto selection = p.selection.getSorted();
                const std::string text0 = p.text.substr(0, selection.first);
                const int x0 = event.fontSystem->getSize(text0, p.size.fontInfo).w;
                const std::string text1 = p.text.substr(0, selection.second);
                const int x1 = event.fontSystem->getSize(text1, p.size.fontInfo).w;
                event.render->drawRect(
                    math::Box2i(g2.x() + x0, g2.y(), x1 - x0, g2.h()),
                    event.style->getColorRole(ColorRole::Checked));
            }

            math::Vector2i pos(
                g2.x(),
                g2.y() + g2.h() / 2 - p.size.fontMetrics.lineHeight / 2 +
                p.size.fontMetrics.ascender);
            if (!p.text.empty() && p.draw.glyphs.empty())
            {
                p.draw.glyphs = event.fontSystem->getGlyphs(p.text, p.size.fontInfo);
                p.draw.glyphsBox = event.fontSystem->getBox(p.text, p.size.fontInfo);
            }
            event.render->drawText(
                p.draw.glyphs,
                pos,
                event.style->getColorRole(enabled ?
                    ColorRole::Text :
                    ColorRole::TextDisabled));
            
            /*for (const auto& box : p.draw.glyphsBox)
            {
                const math::Box2i box2(
                    g2.x() + box.x(),
                    g2.y() + g2.h() / 2 - p.size.fontMetrics.lineHeight / 2 + box.y(),
                    box.w(),
                    box.h());
                event.render->drawRect(
                    box2,
                    dtk::Color4F(1.F, 0.F, 0.F, .2F));
            }*/

            if (p.cursorVisible)
            {
                const std::string text = p.text.substr(0, p.cursorPos);
                const int x = event.fontSystem->getSize(text, p.size.fontInfo).w;
                event.render->drawRect(
                    math::Box2i(
                        g2.x() + x,
                        g2.y(),
                        p.size.border,
                        g2.h()),
                    event.style->getColorRole(ColorRole::Text));
            }
        }

        void LineEdit::mouseMoveEvent(MouseMoveEvent& event)
        {
            IWidget::mouseMoveEvent(event);
            TLRENDER_P();
            if (_mouse.press)
            {
                const int cursorPos = _getCursorPos(event.pos);
                if (cursorPos != p.cursorPos)
                {
                    p.cursorPos = cursorPos;
                    p.cursorVisible = true;
                    p.cursorTimer = std::chrono::steady_clock::now();
                    _updates |= Update::Draw;
                }
                if (cursorPos != p.selection.get().second)
                {
                    p.selection.setSecond(cursorPos);
                    _updates |= Update::Draw;
                }
            }
        }

        void LineEdit::mousePressEvent(MouseClickEvent& event)
        {
            IWidget::mousePressEvent(event);
            TLRENDER_P();
            const int cursorPos = _getCursorPos(event.pos);
            if (cursorPos != p.cursorPos)
            {
                p.cursorPos = cursorPos;
                p.cursorVisible = true;
                p.cursorTimer = std::chrono::steady_clock::now();
                _updates |= Update::Draw;
            }
            const SelectionPair selection(cursorPos, cursorPos);
            if (selection != p.selection.get())
            {
                p.selection.set(selection);
                _updates |= Update::Draw;
            }
            takeKeyFocus();
        }

        void LineEdit::keyFocusEvent(bool value)
        {
            IWidget::keyFocusEvent(value);
            TLRENDER_P();
            if (!value)
            {
                p.selection.clear();
                if (p.textCallback)
                {
                    p.textCallback(p.text);
                }
                _updates |= Update::Draw;
            }
            if (p.focusCallback)
            {
                p.focusCallback(value);
            }
        }

        void LineEdit::keyPressEvent(KeyEvent& event)
        {
            TLRENDER_P();
            switch (event.key)
            {
            case Key::Up:
            case Key::Down:
            case Key::PageUp:
            case Key::PageDown:
            case Key::Tab:
                break;
            default: event.accept = true; break;
            }
            switch (event.key)
            {
            case Key::A:
                if (event.modifiers & static_cast<int>(ui::KeyModifier::Control))
                {
                    p.selection.clear();
                    p.selection.select(0, p.text.size());
                    _updates |= Update::Draw;
                }
                break;
            case Key::C:
                if (event.modifiers & static_cast<int>(ui::KeyModifier::Control))
                {
                    if (p.selection.isValid())
                    {
                        if (auto window = getWindow())
                        {
                            if (auto clipboard = window->getClipboard())
                            {
                                const auto selection = p.selection.getSorted();
                                const std::string text = p.text.substr(
                                    selection.first,
                                    selection.second - selection.first);
                                clipboard->setText(text);
                            }
                        }
                    }
                }
                break;
            case Key::V:
                if (event.modifiers & static_cast<int>(ui::KeyModifier::Control))
                {
                    if (auto window = getWindow())
                    {
                        if (auto clipboard = window->getClipboard())
                        {
                            const std::string text = clipboard->getText();
                            if (p.selection.isValid())
                            {
                                const auto selection = p.selection.getSorted();
                                p.text.replace(
                                    selection.first,
                                    selection.second - selection.first,
                                    text);
                                p.selection.clear();
                                p.cursorPos = selection.first + text.size();
                            }
                            else
                            {
                                p.text.insert(p.cursorPos, text);
                                p.cursorPos += text.size();
                            }
                            if (p.textChangedCallback)
                            {
                                p.textChangedCallback(p.text);
                            }
                            _textUpdate();
                        }
                    }
                }
                break;
            case Key::X:
                if (event.modifiers & static_cast<int>(ui::KeyModifier::Control))
                {
                    if (p.selection.isValid())
                    {
                        if (auto window = getWindow())
                        {
                            if (auto clipboard = window->getClipboard())
                            {
                                const auto selection = p.selection.getSorted();
                                const std::string text = p.text.substr(
                                    selection.first,
                                    selection.second - selection.first);
                                clipboard->setText(text);
                                p.text.replace(
                                    selection.first,
                                    selection.second - selection.first,
                                    "");
                                p.selection.clear();
                                p.cursorPos = selection.first;
                                if (p.textChangedCallback)
                                {
                                    p.textChangedCallback(p.text);
                                }
                                _textUpdate();
                            }
                        }
                    }
                }
                break;
            case Key::Left:
                if (p.cursorPos > 0)
                {
                    if (event.modifiers & static_cast<int>(ui::KeyModifier::Shift))
                    {
                        p.selection.select(p.cursorPos, p.cursorPos - 1);
                    }
                    else
                    {
                        p.selection.clear();
                    }

                    p.cursorPos--;
                    p.cursorVisible = true;
                    p.cursorTimer = std::chrono::steady_clock::now();
                    
                    _updates |= Update::Draw;
                }
                break;
            case Key::Right:
                if (p.cursorPos < p.text.size())
                {
                    if (event.modifiers & static_cast<int>(ui::KeyModifier::Shift))
                    {
                        p.selection.select(p.cursorPos, p.cursorPos + 1);
                    }
                    else
                    {
                        p.selection.clear();
                    }

                    p.cursorPos++;
                    p.cursorVisible = true;
                    p.cursorTimer = std::chrono::steady_clock::now();

                    _updates |= Update::Draw;
                }
                break;
            case Key::Home:
                if (p.cursorPos > 0)
                {
                    if (event.modifiers & static_cast<int>(ui::KeyModifier::Shift))
                    {
                        p.selection.select(p.cursorPos, 0);
                    }
                    else
                    {
                        p.selection.clear();
                    }

                    p.cursorPos = 0;
                    p.cursorVisible = true;
                    p.cursorTimer = std::chrono::steady_clock::now();

                    _updates |= Update::Draw;
                }
                break;
            case Key::End:
                if (p.cursorPos < p.text.size())
                {
                    if (event.modifiers & static_cast<int>(ui::KeyModifier::Shift))
                    {
                        p.selection.select(p.cursorPos, p.text.size());
                    }
                    else
                    {
                        p.selection.clear();
                    }

                    p.cursorPos = p.text.size();
                    p.cursorVisible = true;
                    p.cursorTimer = std::chrono::steady_clock::now();

                    _updates |= Update::Draw;
                }
                break;
            case Key::Backspace:
                if (p.selection.isValid())
                {
                    const auto selection = p.selection.getSorted();
                    p.text.erase(
                        selection.first,
                        selection.second - selection.first);
                    p.selection.clear();
                    p.cursorPos = selection.first;
                    if (p.textChangedCallback)
                    {
                        p.textChangedCallback(p.text);
                    }
                    _textUpdate();
                }
                else if (p.cursorPos > 0)
                {
                    const auto i = p.text.begin() + p.cursorPos - 1;
                    p.text.erase(i);
                    p.cursorPos--;
                    if (p.textChangedCallback)
                    {
                        p.textChangedCallback(p.text);
                    }
                    _textUpdate();
                }
                break;
            case Key::Delete:
                if (p.selection.isValid())
                {
                    const auto selection = p.selection.getSorted();
                    p.text.erase(
                        selection.first,
                        selection.second - selection.first);
                    p.selection.clear();
                    p.cursorPos = selection.first;
                    if (p.textChangedCallback)
                    {
                        p.textChangedCallback(p.text);
                    }
                    _textUpdate();
                }
                else if (p.cursorPos < p.text.size())
                {
                    const auto i = p.text.begin() + p.cursorPos;
                    p.text.erase(i);
                    if (p.textChangedCallback)
                    {
                        p.textChangedCallback(p.text);
                    }
                    _textUpdate();
                }
                break;
            case Key::Enter:
                if (p.textCallback)
                {
                    p.textCallback(p.text);
                }
                break;
            case Key::Escape:
                if (hasKeyFocus())
                {
                    releaseKeyFocus();
                }
                break;
            default: break;
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
            if (p.selection.isValid())
            {
                const auto selection = p.selection.getSorted();
                p.text.replace(
                    selection.first,
                    selection.second - selection.first,
                    event.text);
                p.selection.clear();
                p.cursorPos = selection.first + event.text.size();
            }
            else
            {
                p.text.insert(p.cursorPos, event.text);
                p.cursorPos += event.text.size();
            }
            if (p.textChangedCallback)
            {
                p.textChangedCallback(p.text);
            }
            _textUpdate();
        }

        math::Box2i LineEdit::_getAlignGeometry() const
        {
            return align(
                _geometry,
                _sizeHint,
                Stretch::Expanding,
                Stretch::Expanding,
                _hAlign,
                _vAlign);
        }

        int LineEdit::_getCursorPos(const math::Vector2i& value)
        {
            TLRENDER_P();
            int out = 0;
            const math::Box2i g = _getAlignGeometry();
            const math::Box2i g2 = g.margin(-p.size.border * 2);
            const math::Vector2i pos(
                math::clamp(value.x, g2.min.x, g2.max.x - 1),
                math::clamp(value.y, g2.min.y, g2.max.y - 1));
            math::Box2i box(
                g2.x(),
                g2.y(),
                0,
                g2.h());
            for (const auto& glyphBox : p.draw.glyphsBox)
            {
                box.max.x = g2.x() + glyphBox.x() + glyphBox.w();
                if (box.contains(pos))
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
            p.size.textInit = true;
            _updates |= Update::Size;
            _updates |= Update::Draw;
        }
    }
}
