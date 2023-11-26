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
                    const std::shared_ptr<IWidget>& parent);

                MenuButton();

            public:
                virtual ~MenuButton();

                static std::shared_ptr<MenuButton> create(
                    const std::shared_ptr<system::Context>&,
                    const std::shared_ptr<IWidget>& parent = nullptr);

                void setShortcut(Key, int modifiers = 0);
                
                void setSubMenuIcon(const std::string&);

                void setText(const std::string&) override;
                void setFontRole(FontRole) override;

                void tickEvent(
                    bool,
                    bool,
                    const TickEvent&) override;
                void sizeHintEvent(const SizeHintEvent&) override;
                void clipEvent(const math::Box2i&, bool) override;
                void drawEvent(
                    const math::Box2i&,
                    const DrawEvent&) override;
                void keyPressEvent(KeyEvent&) override;
                void keyReleaseEvent(KeyEvent&) override;

            private:
                Key _shortcut = Key::Unknown;
                int _shortcutModifiers = 0;
                std::string _shortcutText;

                float _iconScale = 1.F;
                struct IconData
                {
                    std::string name;
                    bool init = false;
                    std::future<std::shared_ptr<image::Image> > future;
                    std::shared_ptr<image::Image> image;
                };
                IconData _checkedIcon;
                IconData _uncheckedIcon;
                IconData _subMenuIcon;

                struct SizeData
                {
                    int sizeInit = true;
                    int margin = 0;
                    int spacing = 0;
                    int border = 0;

                    bool textInit = true;
                    image::FontInfo fontInfo;
                    image::FontMetrics fontMetrics;
                    math::Size2i textSize;
                    math::Size2i shortcutSize;
                };
                SizeData _size;

                struct DrawData
                {
                    std::vector<std::shared_ptr<image::Glyph> > textGlyphs;
                    std::vector<std::shared_ptr<image::Glyph> > shortcutGlyphs;
                };
                DrawData _draw;
            };

            void MenuButton::_init(
                const std::shared_ptr<system::Context>& context,
                const std::shared_ptr<IWidget>& parent)
            {
                IButton::_init("tl::ui::MenuButton", context, parent);
                
                setButtonRole(ColorRole::None);
                setAcceptsKeyFocus(true);
                
                _checkedIcon.name = "MenuChecked";
                _checkedIcon.init = true;
                _uncheckedIcon.name = "MenuUnchecked";
                _uncheckedIcon.init = true;
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
                _size.textInit = true;
                _updates |= Update::Size;
                _updates |= Update::Draw;
            }
        
            void MenuButton::setSubMenuIcon(const std::string& name)
            {
                _subMenuIcon.name = name;
                _subMenuIcon.init = true;
                _subMenuIcon.image.reset();
            }

            void MenuButton::setText(const std::string& value)
            {
                const bool changed = value != _text;
                IButton::setText(value);
                if (changed)
                {
                    _size.textInit = true;
                    _updates |= Update::Size;
                    _updates |= Update::Draw;
                }
            }

            void MenuButton::setFontRole(FontRole value)
            {
                const bool changed = value != _fontRole;
                IButton::setFontRole(value);
                if (changed)
                {
                    _size.textInit = true;
                    _updates |= Update::Size;
                    _updates |= Update::Draw;
                }
            }

            void MenuButton::tickEvent(
                bool parentsVisible,
                bool parentsEnabled,
                const TickEvent& event)
            {
                IButton::tickEvent(parentsVisible, parentsEnabled, event);
                if (_displayScale != _iconScale)
                {
                    _iconScale = _displayScale;
                    _checkedIcon.init = true;
                    _checkedIcon.future = std::future<std::shared_ptr<image::Image> >();
                    _checkedIcon.image.reset();
                    _uncheckedIcon.init = true;
                    _uncheckedIcon.future = std::future<std::shared_ptr<image::Image> >();
                    _uncheckedIcon.image.reset();
                    _subMenuIcon.init = true;
                    _subMenuIcon.future = std::future<std::shared_ptr<image::Image> >();
                    _subMenuIcon.image.reset();
                }
                if (!_checkedIcon.name.empty() && _checkedIcon.init)
                {
                    _checkedIcon.init = false;
                    _checkedIcon.future = event.iconLibrary->request(_checkedIcon.name, _displayScale);
                }
                if (_checkedIcon.future.valid() &&
                    _checkedIcon.future.wait_for(std::chrono::seconds(0)) == std::future_status::ready)
                {
                    _checkedIcon.image = _checkedIcon.future.get();
                    _updates |= Update::Size;
                    _updates |= Update::Draw;
                }
                if (!_uncheckedIcon.name.empty() && _uncheckedIcon.init)
                {
                    _uncheckedIcon.init = false;
                    _uncheckedIcon.future = event.iconLibrary->request(_uncheckedIcon.name, _displayScale);
                }
                if (_uncheckedIcon.future.valid() &&
                    _uncheckedIcon.future.wait_for(std::chrono::seconds(0)) == std::future_status::ready)
                {
                    _uncheckedIcon.image = _uncheckedIcon.future.get();
                    _updates |= Update::Size;
                    _updates |= Update::Draw;
                }
                if (!_subMenuIcon.name.empty() && _subMenuIcon.init)
                {
                    _subMenuIcon.init = false;
                    _subMenuIcon.future = event.iconLibrary->request(_subMenuIcon.name, _displayScale);
                }
                if (_subMenuIcon.future.valid() &&
                    _subMenuIcon.future.wait_for(std::chrono::seconds(0)) == std::future_status::ready)
                {
                    _subMenuIcon.image = _subMenuIcon.future.get();
                    _updates |= Update::Size;
                    _updates |= Update::Draw;
                }
            }

            void MenuButton::sizeHintEvent(const SizeHintEvent& event)
            {
                const bool displayScaleChanged = event.displayScale != _displayScale;
                IButton::sizeHintEvent(event);

                if (displayScaleChanged || _size.sizeInit)
                {
                    _size.margin = event.style->getSizeRole(SizeRole::MarginInside, _displayScale);
                    _size.spacing = event.style->getSizeRole(SizeRole::SpacingSmall, _displayScale);
                    _size.border = event.style->getSizeRole(SizeRole::Border, _displayScale);
                }
                if (displayScaleChanged || _size.textInit || _size.sizeInit)
                {
                    _size.fontInfo = event.style->getFontRole(_fontRole, _displayScale);
                    _size.fontMetrics = event.fontSystem->getMetrics(_size.fontInfo);
                    _size.textSize = event.fontSystem->getSize(_text, _size.fontInfo);
                    _size.shortcutSize = event.fontSystem->getSize(_shortcutText, _size.fontInfo);
                    _draw.textGlyphs.clear();
                    _draw.shortcutGlyphs.clear();
                }
                _size.sizeInit = false;
                _size.textInit = false;

                _sizeHint = math::Size2i();
                if (_iconImage)
                {
                    _sizeHint.w = _iconImage->getWidth() + _size.spacing;
                    _sizeHint.h = _iconImage->getHeight();
                }
                else if (_checked && _checkedIcon.image)
                {
                    _sizeHint.w = _checkedIcon.image->getWidth() + _size.spacing;
                    _sizeHint.h = _checkedIcon.image->getHeight();
                }
                else if (!_checked && _uncheckedIcon.image)
                {
                    _sizeHint.w = _uncheckedIcon.image->getWidth() + _size.spacing;
                    _sizeHint.h = _uncheckedIcon.image->getHeight();
                }
                if (!_text.empty())
                {
                    _sizeHint.w += _size.textSize.w + _size.margin * 2;
                    _sizeHint.h = std::max(_sizeHint.h, _size.fontMetrics.lineHeight);
                }
                if (!_shortcutText.empty())
                {
                    _sizeHint.w += _size.spacing * 4 + _size.shortcutSize.w;
                    _sizeHint.h = std::max(_sizeHint.h, _size.shortcutSize.h);
                }
                if (_subMenuIcon.image)
                {
                    _sizeHint.w += _size.spacing + _subMenuIcon.image->getWidth();
                    _sizeHint.h = std::max(_sizeHint.h, _subMenuIcon.image->getHeight());
                }
                _sizeHint.w +=
                    _size.margin * 2 +
                    _size.border * 4;
                _sizeHint.h +=
                    _size.margin * 2 +
                    _size.border * 4;
            }

            void MenuButton::clipEvent(const math::Box2i& clipRect, bool clipped)
            {
                IWidget::clipEvent(clipRect, clipped);
                if (clipped)
                {
                    _draw.textGlyphs.clear();
                    _draw.shortcutGlyphs.clear();
                }
            }

            void MenuButton::drawEvent(
                const math::Box2i& drawRect,
                const DrawEvent& event)
            {
                IButton::drawEvent(drawRect, event);

                const math::Box2i& g = _geometry;
                const bool enabled = isEnabled();

                // Draw the key focus.
                if (_keyFocus)
                {
                    event.render->drawMesh(
                        border(g, _size.border * 2),
                        math::Vector2i(),
                        event.style->getColorRole(ColorRole::KeyFocus));
                }

                // Draw the background.
                if (_buttonRole != ColorRole::None)
                {
                    event.render->drawRect(
                        g,
                        event.style->getColorRole(_buttonRole));
                }
                
                // Draw the pressed and hover states.
                if (_mouse.press && _geometry.contains(_mouse.pos))
                {
                    event.render->drawRect(
                        g,
                        event.style->getColorRole(ColorRole::Pressed));
                }
                else if (_mouse.inside)
                {
                    event.render->drawRect(
                        g,
                        event.style->getColorRole(ColorRole::Hover));
                }

                // Draw the icon.
                const math::Box2i g2 = g.margin(-_size.border * 2);
                int x = g2.x() + _size.margin;
                if (_iconImage)
                {
                    if (_checked)
                    {
                        event.render->drawRect(
                            math::Box2i(g2.x(), g2.y(), g2.h(), g2.h()),
                            event.style->getColorRole(ColorRole::Checked));
                    }
                    const image::Size& iconSize = _iconImage->getSize();
                    event.render->drawImage(
                        _iconImage,
                        math::Box2i(
                            x,
                            g2.y() + g2.h() / 2 - iconSize.h / 2,
                            iconSize.w,
                            iconSize.h),
                        event.style->getColorRole(enabled ?
                            ColorRole::Text :
                            ColorRole::TextDisabled));
                    x += iconSize.w + _size.spacing;
                }
                else if (_checked && _checkedIcon.image)
                {
                    event.render->drawRect(
                        math::Box2i(g2.x(), g2.y(), g2.h(), g2.h()),
                        event.style->getColorRole(ColorRole::Checked));
                    const image::Size& iconSize = _checkedIcon.image->getSize();
                    event.render->drawImage(
                        _checkedIcon.image,
                        math::Box2i(
                            x,
                            g2.y() + g2.h() / 2 - iconSize.h / 2,
                            iconSize.w,
                            iconSize.h),
                        event.style->getColorRole(enabled ?
                            ColorRole::Text :
                            ColorRole::TextDisabled));
                    x += iconSize.w + _size.spacing;
                }
                else if (!_checked && _uncheckedIcon.image)
                {
                    const image::Size& iconSize = _uncheckedIcon.image->getSize();
                    event.render->drawImage(
                        _uncheckedIcon.image,
                        math::Box2i(
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
                        g2.y() + g2.h() / 2 - _size.textSize.h / 2 +
                        _size.fontMetrics.ascender);
                    event.render->drawText(
                        _draw.textGlyphs,
                        pos,
                        event.style->getColorRole(enabled ?
                            ColorRole::Text :
                            ColorRole::TextDisabled));
                }

                // Draw the shortcut.
                if (!_shortcutText.empty())
                {
                    if (_draw.shortcutGlyphs.empty())
                    {
                        _draw.shortcutGlyphs = event.fontSystem->getGlyphs(_shortcutText, _size.fontInfo);
                    }
                    const math::Vector2i pos(
                        g2.max.x - _size.margin - _size.shortcutSize.w,
                        g2.y() + g2.h() / 2 - _size.shortcutSize.h / 2 +
                        _size.fontMetrics.ascender);
                    event.render->drawText(
                        _draw.shortcutGlyphs,
                        pos,
                        event.style->getColorRole(enabled ?
                            ColorRole::Text :
                            ColorRole::TextDisabled));
                }

                // Draw the sub menu icon.
                if (_subMenuIcon.image)
                {
                    const image::Size& iconSize = _subMenuIcon.image->getSize();
                    event.render->drawImage(
                      _subMenuIcon.image,
                      math::Box2i(
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
                if (0 == event.modifiers)
                {
                    switch (event.key)
                    {
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
            }

            void MenuButton::keyReleaseEvent(KeyEvent& event)
            {
                event.accept = true;
            }
        }

        struct Menu::Private
        {
            std::list<std::shared_ptr<Action> > items;
            std::map<std::shared_ptr<Action>, std::shared_ptr<MenuButton> > buttons;
            std::shared_ptr<VerticalLayout> layout;
        };

        void Menu::_init(
            const std::shared_ptr<system::Context>& context,
            const std::shared_ptr<IWidget>& parent)
        {
            IMenuPopup::_init("tl::ui::Menu", context, parent);
            TLRENDER_P();
            p.layout = VerticalLayout::create(context);
            p.layout->setSpacingRole(SizeRole::None);
            setWidget(p.layout);
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

        void Menu::addItem(const std::shared_ptr<Action>& item)
        {
            TLRENDER_P();
            p.items.push_back(item);
            if (auto context = _context.lock())
            {
                auto button = MenuButton::create(context);
                button->setText(item->text);
                button->setIcon(item->icon);
                button->setShortcut(item->shortcut, item->shortcutModifiers);
                button->setClickedCallback(
                    [this, item]
                    {
                        if (item->callback)
                        {
                            item->callback();
                        }
                        close();
                    });
                button->setCheckable(item->checkable);
                button->setChecked(item->checked);
                button->setCheckedCallback(
                    [this, item](bool value)
                    {
                        if (item->checkedCallback)
                        {
                            item->checkedCallback(value);
                        }
                        close();
                    });
                button->setParent(p.layout);
                p.buttons[item] = button;
            }
        }

        void Menu::setItemChecked(const std::shared_ptr<Action>& item, bool value)
        {
            TLRENDER_P();
            const auto i = std::find(p.items.begin(), p.items.end(), item);
            if (i != p.items.end())
            {
                i->get()->checked = value;
            }
            const auto j = p.buttons.find(item);
            if (j != p.buttons.end())
            {
                j->second->setChecked(value);
            }
        }

        void Menu::setItemEnabled(const std::shared_ptr<Action>& item, bool value)
        {
            TLRENDER_P();
            const auto i = p.buttons.find(item);
            if (i != p.buttons.end())
            {
                i->second->setEnabled(value);
            }
        }

        std::shared_ptr<Menu> Menu::addSubMenu(const std::string& text)
        {
            TLRENDER_P();
            std::shared_ptr<Menu> out;
            if (auto context = _context.lock())
            {
                out = Menu::create(context);
                out->setPopupStyle(MenuPopupStyle::SubMenu);

                auto button = MenuButton::create(context);
                button->setText(text);
                button->setSubMenuIcon("SubMenuArrow");
                button->setPressedCallback(
                    [this, out, button]
                    {
                        if (!out->isOpen())
                        {
                            out->open(getWindow(), button->getGeometry());
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
            p.items.clear();
            for (auto button : p.buttons)
            {
                button.second->setParent(nullptr);
            }
            p.buttons.clear();
        }

        bool Menu::shortcut(Key shortcut, int modifiers)
        {
            TLRENDER_P();
            bool out = false;
            for (const auto& item : p.items)
            {
                if (shortcut == item->shortcut &&
                    modifiers == item->shortcutModifiers)
                {
                    if (item->callback)
                    {
                        item->callback();
                        out = true;
                    }
                    if (item->checkedCallback)
                    {
                        setItemChecked(item, !item->checked);
                        item->checkedCallback(item->checked);
                        out = true;
                    }
                }
            }
            return out;
        }
    }
}
