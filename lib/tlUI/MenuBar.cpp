// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2023 Darby Johnston
// All rights reserved.

#include <tlUI/MenuBar.h>

#include <tlUI/DrawUtil.h>
#include <tlUI/EventLoop.h>
#include <tlUI/ListButton.h>
#include <tlUI/RowLayout.h>

namespace tl
{
    namespace ui
    {
        namespace
        {
            /*class MenuBarButton : public IButton
            {
                TLRENDER_NON_COPYABLE(MenuBarButton);

            protected:
                void _init(
                    const std::shared_ptr<system::Context>&,
                    const std::shared_ptr<IWidget>& parent = nullptr);

                MenuBarButton();

            public:
                ~MenuBarButton() override;

                static std::shared_ptr<MenuBarButton> create(
                    const std::shared_ptr<system::Context>&,
                    const std::shared_ptr<IWidget>& parent = nullptr);

                void setText(const std::string&) override;

                bool acceptsKeyFocus() const override;
                void sizeHintEvent(const SizeHintEvent&) override;
                void clipEvent(
                    const math::BBox2i&,
                    bool,
                    const ClipEvent&) override;
                void drawEvent(
                    const math::BBox2i&,
                    const DrawEvent&) override;
                void keyPressEvent(KeyEvent&) override;
                void keyReleaseEvent(KeyEvent&) override;

            private:
                struct SizeData
                {
                    int margin = 0;
                    int spacing = 0;
                    int border = 0;
                    imaging::FontInfo fontInfo;
                    imaging::FontMetrics fontMetrics;
                    math::Vector2i textSize;
                };
                SizeData _size;

                struct DrawData
                {
                    std::vector<std::shared_ptr<imaging::Glyph> > glyphs;
                };
                DrawData _draw;
            };

            void MenuBarButton::_init(
                const std::shared_ptr<system::Context>& context,
                const std::shared_ptr<IWidget>& parent)
            {
                IButton::_init("tl::ui::MenuBarButton", context, parent);
                setButtonRole(ColorRole::None);
            }

            MenuBarButton::MenuBarButton()
            {}

            MenuBarButton::~MenuBarButton()
            {}

            std::shared_ptr<MenuBarButton> MenuBarButton::create(
                const std::shared_ptr<system::Context>& context,
                const std::shared_ptr<IWidget>& parent)
            {
                auto out = std::shared_ptr<MenuBarButton>(new MenuBarButton);
                out->_init(context, parent);
                return out;
            }

            void MenuBarButton::setText(const std::string& value)
            {
                const bool changed = value != _text;
                IButton::setText(value);
                if (changed)
                {
                    _draw.glyphs.clear();
                }
            }

            bool MenuBarButton::acceptsKeyFocus() const
            {
                return true;
            }

            void MenuBarButton::sizeHintEvent(const SizeHintEvent& event)
            {
                IButton::sizeHintEvent(event);

                _size.margin = event.style->getSizeRole(SizeRole::MarginInside, event.displayScale);
                _size.spacing = event.style->getSizeRole(SizeRole::SpacingSmall, event.displayScale);
                _size.border = event.style->getSizeRole(SizeRole::Border, event.displayScale);

                _sizeHint = math::Vector2i();
                if (!_text.empty())
                {
                    _size.fontMetrics = event.getFontMetrics(_fontRole);
                    auto fontInfo = event.style->getFontRole(_fontRole, event.displayScale);
                    _size.fontInfo = fontInfo;
                    _size.textSize = event.fontSystem->getSize(_text, fontInfo);

                    _sizeHint.x = _size.textSize.x + _size.margin * 2;
                    _sizeHint.y = _size.fontMetrics.lineHeight;
                }
                if (_iconImage)
                {
                    _sizeHint.x += _iconImage->getWidth();
                    if (!_text.empty())
                    {
                        _sizeHint.x += _size.spacing;
                    }
                    _sizeHint.y = std::max(
                        _sizeHint.y,
                        static_cast<int>(_iconImage->getHeight()));
                }
                _sizeHint.x +=
                    _size.margin * 2 +
                    _size.border * 4;
                _sizeHint.y +=
                    _size.margin * 2 +
                    _size.border * 4;
            }

            void MenuBarButton::clipEvent(
                const math::BBox2i& clipRect,
                bool clipped,
                const ClipEvent& event)
            {
                IWidget::clipEvent(clipRect, clipped, event);
                if (clipped)
                {
                    _draw.glyphs.clear();
                }
            }

            void MenuBarButton::drawEvent(
                const math::BBox2i& drawRect,
                const DrawEvent& event)
            {
                IButton::drawEvent(drawRect, event);

                const math::BBox2i& g = _geometry;
                const bool enabled = isEnabled();

                // Draw the key focus.
                if (_keyFocus)
                {
                    event.render->drawMesh(
                        border(g, _size.border * 2),
                        math::Vector2i(),
                        event.style->getColorRole(ColorRole::KeyFocus));
                }

                // Draw the background and checked state.
                const ColorRole colorRole = _checked ?
                    ColorRole::Checked :
                    _buttonRole;
                if (colorRole != ColorRole::None)
                {
                    event.render->drawRect(
                        g,
                        event.style->getColorRole(colorRole));
                }

                // Draw the pressed and hover states.
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

                // Draw the icon.
                const math::BBox2i g2 = g.margin(-_size.border * 2);
                int x = g2.x() + _size.margin;
                if (_iconImage)
                {
                    const imaging::Size& iconSize = _iconImage->getSize();
                    event.render->drawImage(
                        _iconImage,
                        math::BBox2i(
                            x,
                            g2.y() + g2.h() / 2 - iconSize.h / 2,
                            iconSize.w,
                            iconSize.h),
                        event.style->getColorRole(enabled ?
                            ColorRole::Text :
                            ColorRole::TextDisabled));
                    x += iconSize.w + _size.spacing;
                }

                // Draw the text.
                if (!_text.empty())
                {
                    if (_draw.glyphs.empty())
                    {
                        _draw.glyphs = event.fontSystem->getGlyphs(_text, _size.fontInfo);
                    }
                    const math::Vector2i pos(
                        x + _size.margin,
                        g2.y() + g2.h() / 2 - _size.textSize.y / 2 +
                        _size.fontMetrics.ascender);
                    event.render->drawText(
                        _draw.glyphs,
                        pos,
                        event.style->getColorRole(enabled ?
                            ColorRole::Text :
                            ColorRole::TextDisabled));
                }
            }

            void MenuBarButton::keyPressEvent(KeyEvent& event)
            {
                switch (event.key)
                {
                case Key::Space:
                case Key::Enter:
                    event.accept = true;
                    _click();
                    break;
                case Key::Escape:
                    if (hasKeyFocus())
                    {
                        event.accept = true;
                        releaseKeyFocus();
                    }
                    break;
                default: break;
                }
            }

            void MenuBarButton::keyReleaseEvent(KeyEvent& event)
            {
                event.accept = true;
            }*/
        }

        struct MenuBar::Private
        {
            std::list<std::shared_ptr<IPopup> > menus;
            std::list<std::shared_ptr<ListButton> > buttons;
            std::shared_ptr<HorizontalLayout> layout;
        };

        void MenuBar::_init(
            const std::shared_ptr<system::Context>& context,
            const std::shared_ptr<IWidget>& parent)
        {
            IWidget::_init("tl::ui::MenuBar", context, parent);
            TLRENDER_P();
            p.layout = HorizontalLayout::create(context, shared_from_this());
            p.layout->setSpacingRole(SizeRole::None);
        }

        MenuBar::MenuBar() :
            _p(new Private)
        {}

        MenuBar::~MenuBar()
        {}

        std::shared_ptr<MenuBar> MenuBar::create(
            const std::shared_ptr<system::Context>& context,
            const std::shared_ptr<IWidget>& parent)
        {
            auto out = std::shared_ptr<MenuBar>(new MenuBar);
            out->_init(context, parent);
            return out;
        }
        
        void MenuBar::addMenu(
            const std::string& text,
            const std::shared_ptr<IPopup>& menu)
        {
            TLRENDER_P();
            p.menus.push_back(menu);
            if (auto context = _context.lock())
            {
                auto button = ListButton::create(context);
                button->setText(text);
                p.buttons.push_back(button);
                button->setParent(p.layout);
                button->setHoveredCallback(
                    [this, menu, button](bool value)
                    {
                        if (value)
                        {
                            std::shared_ptr<IPopup> openMenu;
                            for (auto& i : _p->menus)
                            {
                                if (i->isOpen())
                                {
                                    openMenu = i;
                                    break;
                                }
                            }
                            if (openMenu && menu != openMenu)
                            {
                                openMenu->close();
                                if (auto eventLoop = getEventLoop().lock())
                                {
                                    button->takeKeyFocus();
                                    menu->open(eventLoop, button->getGeometry());
                                }
                            }
                        }
                    });
                button->setPressedCallback(
                    [this, menu, button]
                    {
                        if (!menu->isOpen())
                        {
                            if (auto eventLoop = getEventLoop().lock())
                            {
                                menu->open(eventLoop, button->getGeometry());
                            }
                        }
                        else
                        {
                            menu->close();
                        }
                    });
                _updates |= Update::Size;
                _updates |= Update::Draw;
            }
        }

        void MenuBar::setGeometry(const math::BBox2i& value)
        {
            IWidget::setGeometry(value);
            TLRENDER_P();
            p.layout->setGeometry(value);
        }

        void MenuBar::sizeHintEvent(const SizeHintEvent& event)
        {
            IWidget::sizeHintEvent(event);
            TLRENDER_P();
            _sizeHint = p.layout->getSizeHint();
        }
    }
}
