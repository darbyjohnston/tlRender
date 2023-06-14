// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2023 Darby Johnston
// All rights reserved.

#include <tlUI/Menu.h>

#include <tlUI/Divider.h>
#include <tlUI/DrawUtil.h>
#include <tlUI/ListButton.h>
#include <tlUI/RowLayout.h>

namespace tl
{
    namespace ui
    {
        namespace
        {
            class MenuButton : public IButton
            {
                TLRENDER_NON_COPYABLE(MenuButton);

            protected:
                void _init(
                    const std::shared_ptr<system::Context>&,
                    const std::shared_ptr<IWidget>& parent = nullptr);

                MenuButton();

            public:
                ~MenuButton() override;

                static std::shared_ptr<MenuButton> create(
                    const std::shared_ptr<system::Context>&,
                    const std::shared_ptr<IWidget>& parent = nullptr);

                void setShortcut(Key, int modifiers = 0);
                
                void setSubMenuIcon(const std::string&);

                void setText(const std::string&) override;
                void setFontRole(FontRole) override;

                bool acceptsKeyFocus() const override;
                void tickEvent(
                    bool,
                    bool,
                    const TickEvent&) override;
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
                Key _shortcut = Key::Unknown;
                int _shortcutModifiers = 0;
                std::string _shortcutText;

                std::string _subMenuIcon;
                float _subMenuIconScale = 1.F;
                bool _subMenuIconInit = false;
                std::future<std::shared_ptr<imaging::Image> > _subMenuIconFuture;
                std::shared_ptr<imaging::Image> _subMenuIconImage;

                struct SizeData
                {
                    int margin = 0;
                    int spacing = 0;
                    int border = 0;
                    imaging::FontInfo fontInfo;
                    imaging::FontMetrics fontMetrics;
                    math::Vector2i textSize;
                    math::Vector2i shortcutSize;
                };
                SizeData _size;

                struct DrawData
                {
                    std::vector<std::shared_ptr<imaging::Glyph> > textGlyphs;
                    std::vector<std::shared_ptr<imaging::Glyph> > shortcutGlyphs;
                };
                DrawData _draw;
            };

            void MenuButton::_init(
                const std::shared_ptr<system::Context>& context,
                const std::shared_ptr<IWidget>& parent)
            {
                IButton::_init("tl::ui::MenuButton", context, parent);
                setButtonRole(ColorRole::None);
            }

            MenuButton::MenuButton()
            {}

            MenuButton::~MenuButton()
            {}

            std::shared_ptr<MenuButton> MenuButton::create(
                const std::shared_ptr<system::Context>& context,
                const std::shared_ptr<IWidget>& parent)
            {
                auto out = std::shared_ptr<MenuButton>(new MenuButton);
                out->_init(context, parent);
                return out;
            }

            void MenuButton::setShortcut(Key key, int modifiers)
            {
                if (key == _shortcut && modifiers == _shortcutModifiers)
                    return;
                _shortcut = key;
                _shortcutModifiers = modifiers;
                _shortcutText = getLabel(_shortcut, _shortcutModifiers);
                _draw.shortcutGlyphs.clear();
                _updates |= Update::Size;
                _updates |= Update::Draw;
            }
        
            void MenuButton::setSubMenuIcon(const std::string& icon)
            {
                _subMenuIcon = icon;
                _subMenuIconInit = true;
                _subMenuIconImage.reset();
            }

            void MenuButton::setText(const std::string& value)
            {
                const bool changed = value != _text;
                IButton::setText(value);
                if (changed)
                {
                    _draw.textGlyphs.clear();
                }
            }

            void MenuButton::setFontRole(FontRole value)
            {
                const bool changed = value != _fontRole;
                IButton::setFontRole(value);
                if (changed)
                {
                    _draw.textGlyphs.clear();
                    _draw.shortcutGlyphs.clear();
                }
            }

            bool MenuButton::acceptsKeyFocus() const
            {
                return true;
            }

            void MenuButton::tickEvent(
                bool parentsVisible,
                bool parentsEnabled,
                const TickEvent& event)
            {
                IButton::tickEvent(parentsVisible, parentsEnabled, event);
                if (event.displayScale != _subMenuIconScale)
                {
                    _subMenuIconScale = event.displayScale;
                    _subMenuIconInit = true;
                    _subMenuIconFuture = std::future<std::shared_ptr<imaging::Image> >();
                    _subMenuIconImage.reset();
                }
                if (!_subMenuIcon.empty() && _subMenuIconInit)
                {
                    _subMenuIconInit = false;
                    _subMenuIconFuture = event.iconLibrary->request(_subMenuIcon, event.displayScale);
                }
                if (_subMenuIconFuture.valid() &&
                    _subMenuIconFuture.wait_for(std::chrono::seconds(0)) == std::future_status::ready)
                {
                    _subMenuIconImage = _subMenuIconFuture.get();
                    _updates |= Update::Size;
                    _updates |= Update::Draw;
                }
            }

            void MenuButton::sizeHintEvent(const SizeHintEvent& event)
            {
                IButton::sizeHintEvent(event);

                _size.margin = event.style->getSizeRole(SizeRole::MarginInside, event.displayScale);
                _size.spacing = event.style->getSizeRole(SizeRole::SpacingSmall, event.displayScale);
                _size.border = event.style->getSizeRole(SizeRole::Border, event.displayScale);

                _size.fontMetrics = event.getFontMetrics(_fontRole);
                auto fontInfo = event.style->getFontRole(_fontRole, event.displayScale);
                _size.fontInfo = fontInfo;

                _sizeHint = math::Vector2i();
                if (_iconImage)
                {
                    _sizeHint.x = _iconImage->getWidth() + _size.spacing;
                    _sizeHint.y = _iconImage->getHeight();
                }
                if (!_text.empty())
                {
                    _size.textSize = event.fontSystem->getSize(_text, fontInfo);

                    _sizeHint.x += _size.textSize.x + _size.margin * 2;
                    _sizeHint.y = std::max(
                        _sizeHint.y,
                        static_cast<int>(_size.fontMetrics.lineHeight));
                }
                if (!_shortcutText.empty())
                {
                    _size.shortcutSize = event.fontSystem->getSize(_shortcutText, fontInfo);

                    _sizeHint.x += _size.spacing * 4 + _size.shortcutSize.x;
                    _sizeHint.y = std::max(_sizeHint.y, _size.shortcutSize.y);
                }
                if (_subMenuIconImage)
                {
                    _sizeHint.x += _size.spacing + _subMenuIconImage->getWidth();
                    _sizeHint.y = std::max(
                        _sizeHint.y,
                        static_cast<int>(_subMenuIconImage->getHeight()));
                }
                _sizeHint.x +=
                    _size.margin * 2 +
                    _size.border * 4;
                _sizeHint.y +=
                    _size.margin * 2 +
                    _size.border * 4;
            }

            void MenuButton::clipEvent(
                const math::BBox2i& clipRect,
                bool clipped,
                const ClipEvent& event)
            {
                IWidget::clipEvent(clipRect, clipped, event);
                if (clipped)
                {
                    _draw.textGlyphs.clear();
                    _draw.shortcutGlyphs.clear();
                }
            }

            void MenuButton::drawEvent(
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
                    if (_draw.textGlyphs.empty())
                    {
                        _draw.textGlyphs = event.fontSystem->getGlyphs(_text, _size.fontInfo);
                    }
                    const math::Vector2i pos(
                        x + _size.margin,
                        g2.y() + g2.h() / 2 - _size.textSize.y / 2 +
                        _size.fontMetrics.ascender);
                    event.render->drawText(
                        _draw.textGlyphs,
                        pos,
                        event.style->getColorRole(enabled ?
                            ColorRole::Text :
                            ColorRole::TextDisabled));
                    x += _size.margin + _size.textSize.x;
                }

                // Draw the shortcut.
                if (!_shortcutText.empty())
                {
                    if (_draw.shortcutGlyphs.empty())
                    {
                        _draw.shortcutGlyphs = event.fontSystem->getGlyphs(_shortcutText, _size.fontInfo);
                    }
                    const math::Vector2i pos(
                        g2.max.x - _size.margin - _size.shortcutSize.x,
                        g2.y() + g2.h() / 2 - _size.shortcutSize.y / 2 +
                        _size.fontMetrics.ascender);
                    event.render->drawText(
                        _draw.shortcutGlyphs,
                        pos,
                        event.style->getColorRole(enabled ?
                            ColorRole::Text :
                            ColorRole::TextDisabled));
                }

                // Draw the sub menu icon.
                if (_subMenuIconImage)
                {
                    const imaging::Size& iconSize = _subMenuIconImage->getSize();
                    event.render->drawImage(
                      _subMenuIconImage,
                      math::BBox2i(
                          g2.max.x - _size.margin - iconSize.w,
                          g2.y() + g2.h() / 2 - iconSize.h / 2,
                          iconSize.w,
                          iconSize.h),
                      event.style->getColorRole(enabled ?
                          ColorRole::Text :
                          ColorRole::TextDisabled));
                }
            }

            void MenuButton::keyPressEvent(KeyEvent& event)
            {
                switch (event.key)
                {
                case Key::Space:
                case Key::Enter:
                    event.accept = true;
                    takeKeyFocus();
                    if (_pressedCallback)
                    {
                        _pressedCallback();
                    }
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

            void MenuButton::keyReleaseEvent(KeyEvent& event)
            {
                event.accept = true;
            }
        }

        struct Menu::Private
        {
            std::list<std::shared_ptr<Action> > actions;
            std::map<std::shared_ptr<Action>, std::shared_ptr<MenuButton> > buttons;
            std::shared_ptr<VerticalLayout> layout;
        };

        void Menu::_init(
            const std::shared_ptr<system::Context>& context,
            const std::shared_ptr<IWidget>& parent)
        {
            IPopup::_init("tl::ui::Menu", context, parent);
            TLRENDER_P();
            p.layout = VerticalLayout::create(context, shared_from_this());
            p.layout->setSpacingRole(SizeRole::None);
        }

        Menu::Menu() :
            _p(new Private)
        {}

        Menu::~Menu()
        {}

        std::shared_ptr<Menu> Menu::create(
            const std::shared_ptr<system::Context>& context,
            const std::shared_ptr<IWidget>& parent)
        {
            auto out = std::shared_ptr<Menu>(new Menu);
            out->_init(context, parent);
            return out;
        }

        void Menu::addAction(const std::shared_ptr<Action>& action)
        {
            TLRENDER_P();
            p.actions.push_back(action);
            if (auto context = _context.lock())
            {
                auto button = MenuButton::create(context);
                button->setText(action->getText());
                button->setShortcut(action->getShortcut(), action->getShortcutModifiers());
                button->setIcon(action->getIcon());
                button->setPressedCallback(
                    [action]
                    {
                        action->doPressedCallback();
                    });
                button->setClickedCallback(
                    [action]
                    {
                        action->doClickedCallback();
                    });
                button->setParent(p.layout);
                p.buttons[action] = button;
            }
        }

        std::shared_ptr<Menu> Menu::addSubMenu(const std::string& text)
        {
            TLRENDER_P();
            std::shared_ptr<Menu> out;
            if (auto context = _context.lock())
            {
                out = Menu::create(context);
                out->setPopupStyle(ui::PopupStyle::SubMenu);

                auto button = MenuButton::create(context);
                button->setText(text);
                button->setSubMenuIcon("SubMenuArrow");
                button->setPressedCallback(
                    [this, out, button]
                    {
                        if (!out->isOpen())
                        {
                            if (auto eventLoop = getEventLoop().lock())
                            {
                                out->open(eventLoop, button->getGeometry());
                            }
                        }
                        else
                        {
                            out->close();
                        }
                    });
                button->setParent(p.layout);
            }
            return out;
        }

        void Menu::addDivider()
        {
            TLRENDER_P();
            if (auto context = _context.lock())
            {
                auto divider = Divider::create(Orientation::Horizontal, context);
                divider->setParent(p.layout);
            }
        }

        void Menu::clear()
        {
            TLRENDER_P();
            for (auto child : _children)
            {
                child->setParent(nullptr);
            }
            p.actions.clear();
            p.buttons.clear();
        }
    }
}
