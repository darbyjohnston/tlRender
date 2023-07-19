// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2023 Darby Johnston
// All rights reserved.

#include <tlUI/ComboBox.h>

#include <tlUI/ButtonGroup.h>
#include <tlUI/DrawUtil.h>
#include <tlUI/EventLoop.h>
#include <tlUI/IMenuPopup.h>
#include <tlUI/ListButton.h>
#include <tlUI/RowLayout.h>

namespace tl
{
    namespace ui
    {
        namespace
        {
            class ComboBoxMenu : public IMenuPopup
            {
                TLRENDER_NON_COPYABLE(ComboBoxMenu);

            protected:
                void _init(
                    const std::vector<ComboBoxItem>&,
                    const std::shared_ptr<system::Context>&,
                    const std::shared_ptr<IWidget>& parent = nullptr);

                ComboBoxMenu();

            public:
                ~ComboBoxMenu() override;

                static std::shared_ptr<ComboBoxMenu> create(
                    const std::vector<ComboBoxItem>&,
                    const std::shared_ptr<system::Context>&,
                    const std::shared_ptr<IWidget>& parent = nullptr);

                void setCallback(const std::function<void(int)>&);

            private:
                std::shared_ptr<ButtonGroup> _buttonGroup;
                std::shared_ptr<VerticalLayout> _layout;
                std::function<void(int)> _callback;
            };

            void ComboBoxMenu::_init(
                const std::vector<ComboBoxItem>& items,
                const std::shared_ptr<system::Context>& context,
                const std::shared_ptr<IWidget>& parent)
            {
                IPopup::_init("tl::ui::ComboBoxMenu", context, parent);

                std::vector<std::shared_ptr<ListButton> > buttons;
                _buttonGroup = ButtonGroup::create(ButtonGroupType::Click, context);
                for (const auto& item : items)
                {
                    auto button = ListButton::create(context, shared_from_this());
                    button->setText(item.text);
                    button->setIcon(item.icon);
                    buttons.push_back(button);
                    _buttonGroup->addButton(button);
                }

                _layout = VerticalLayout::create(context, shared_from_this());
                _layout->setSpacingRole(SizeRole::None);
                for (const auto& button : buttons)
                {
                    button->setParent(_layout);
                }
                
                auto weak = std::weak_ptr<ComboBoxMenu>(std::dynamic_pointer_cast<ComboBoxMenu>(shared_from_this()));
                _buttonGroup->setClickedCallback(
                    [weak](int value)
                    {
                        if (auto widget = weak.lock())
                        {
                            if (widget->_callback)
                            {
                                widget->_callback(value);
                            }
                        }
                    });
            }

            ComboBoxMenu::ComboBoxMenu()
            {}

            ComboBoxMenu::~ComboBoxMenu()
            {}

            std::shared_ptr<ComboBoxMenu> ComboBoxMenu::create(
                const std::vector<ComboBoxItem>& items,
                const std::shared_ptr<system::Context>& context,
                const std::shared_ptr<IWidget>& parent)
            {
                auto out = std::shared_ptr<ComboBoxMenu>(new ComboBoxMenu);
                out->_init(items, context, parent);
                return out;
            }

            void ComboBoxMenu::setCallback(const std::function<void(int)>& value)
            {
                _callback = value;
            }
        }

        bool ComboBoxItem::operator == (const ComboBoxItem& other) const
        {
            return text == other.text && icon == other.icon;
        }

        bool ComboBoxItem::operator != (const ComboBoxItem& other) const
        {
            return !(*this == other);
        }

        struct ComboBox::Private
        {
            std::vector<ComboBoxItem> items;
            int currentIndex = -1;
            std::function<void(int)> indexCallback;
            std::function<void(const ComboBoxItem&)> itemCallback;
            FontRole fontRole = FontRole::Label;

            std::string text;
            std::string icon;
            float iconScale = 1.F;
            bool iconInit = false;
            std::future<std::shared_ptr<imaging::Image> > iconFuture;
            std::shared_ptr<imaging::Image> iconImage;
            bool arrowIconInit = false;
            std::future<std::shared_ptr<imaging::Image> > arrowIconFuture;
            std::shared_ptr<imaging::Image> arrowIconImage;

            std::shared_ptr<ComboBoxMenu> menu;

            struct SizeData
            {
                int margin = 0;
                int spacing = 0;
                int border = 0;
                imaging::FontInfo fontInfo;
                imaging::FontMetrics fontMetrics;
                math::Vector2i textSize;
            };
            SizeData size;

            struct DrawData
            {
                std::vector<std::shared_ptr<imaging::Glyph> > glyphs;
            };
            DrawData draw;

            struct MouseData
            {
                bool inside = false;
                math::Vector2i cursorPos;
                bool pressed = false;
            };
            MouseData mouse;
        };

        void ComboBox::_init(
            const std::shared_ptr<system::Context>& context,
            const std::shared_ptr<IWidget>& parent)
        {
            IWidget::_init("tl::ui::ComboBox", context, parent);
            setMouseHover(true);
            setAcceptsKeyFocus(true);
        }

        ComboBox::ComboBox() :
            _p(new Private)
        {}

        ComboBox::~ComboBox()
        {}

        std::shared_ptr<ComboBox> ComboBox::create(
            const std::shared_ptr<system::Context>& context,
            const std::shared_ptr<IWidget>& parent)
        {
            auto out = std::shared_ptr<ComboBox>(new ComboBox);
            out->_init(context, parent);
            return out;
        }

        void ComboBox::setItems(const std::vector<ComboBoxItem>& value)
        {
            TLRENDER_P();
            if (value == p.items)
                return;
            p.items = value;
            p.currentIndex = math::clamp(
                p.currentIndex,
                0,
                static_cast<int>(p.items.size()) - 1);
            const ComboBoxItem item = _getItem(p.currentIndex);
            p.text = item.text;
            p.icon = item.icon;
            p.iconInit = true;
            p.iconFuture = std::future<std::shared_ptr<imaging::Image> >();
            p.iconImage.reset();
            p.draw.glyphs.clear();
            _updates |= Update::Size;
            _updates |= Update::Draw;
        }

        void ComboBox::setItems(const std::vector<std::string>& value)
        {
            TLRENDER_P();
            std::vector<ComboBoxItem> items;
            for (const auto& s : value)
            {
                items.push_back({ s, std::string()});
            }
            setItems(items);
        }

        void ComboBox::setCurrentIndex(int value)
        {
            TLRENDER_P();
            const int tmp = math::clamp(
                value,
                0,
                static_cast<int>(p.items.size()) - 1);
            if (tmp == p.currentIndex)
                return;
            p.currentIndex = tmp;
            const ComboBoxItem item = _getItem(p.currentIndex);
            p.text = item.text;
            p.icon = item.icon;
            p.iconInit = true;
            p.iconFuture = std::future<std::shared_ptr<imaging::Image> >();
            p.iconImage.reset();
            p.draw.glyphs.clear();
            _updates |= Update::Size;
            _updates |= Update::Draw;
        }

        void ComboBox::setIndexCallback(const std::function<void(int)>& value)
        {
            _p->indexCallback = value;
        }

        void ComboBox::setItemCallback(const std::function<void(const ComboBoxItem&)>& value)
        {
            _p->itemCallback = value;
        }

        void ComboBox::setFontRole(FontRole value)
        {
            TLRENDER_P();
            if (value == p.fontRole)
                return;
            p.fontRole = value;
            p.draw.glyphs.clear();
            _updates |= Update::Size;
            _updates |= Update::Draw;
        }

        void ComboBox::setVisible(bool value)
        {
            const bool changed = value != _visible;
            IWidget::setVisible(value);
            if (changed && !_visible)
            {
                _resetMouse();
            }
        }

        void ComboBox::setEnabled(bool value)
        {
            const bool changed = value != _enabled;
            IWidget::setEnabled(value);
            if (changed && !_enabled)
            {
                _resetMouse();
            }
        }

        void ComboBox::tickEvent(
            bool parentsVisible,
            bool parentsEnabled,
            const TickEvent& event)
        {
            IWidget::tickEvent(parentsVisible, parentsEnabled, event);
            TLRENDER_P();
            if (event.displayScale != p.iconScale)
            {
                p.iconScale = event.displayScale;
                p.iconInit = true;
                p.iconFuture = std::future<std::shared_ptr<imaging::Image> >();
                p.iconImage.reset();
                p.arrowIconInit = true;
                p.arrowIconFuture = std::future<std::shared_ptr<imaging::Image> >();
                p.arrowIconImage.reset();
            }
            if (!p.icon.empty() && p.iconInit)
            {
                p.iconInit = false;
                p.iconFuture = event.iconLibrary->request(p.icon, event.displayScale);
            }
            if (p.iconFuture.valid() &&
                p.iconFuture.wait_for(std::chrono::seconds(0)) == std::future_status::ready)
            {
                p.iconImage = p.iconFuture.get();
                _updates |= Update::Size;
                _updates |= Update::Draw;
            }
            if (p.arrowIconInit)
            {
                p.arrowIconInit = false;
                p.arrowIconFuture = event.iconLibrary->request("MenuArrow", event.displayScale);
            }
            if (p.arrowIconFuture.valid() &&
                p.arrowIconFuture.wait_for(std::chrono::seconds(0)) == std::future_status::ready)
            {
                p.arrowIconImage = p.arrowIconFuture.get();
                _updates |= Update::Size;
                _updates |= Update::Draw;
            }
        }

        void ComboBox::sizeHintEvent(const SizeHintEvent& event)
        {
            IWidget::sizeHintEvent(event);
            TLRENDER_P();

            p.size.margin = event.style->getSizeRole(SizeRole::MarginInside, event.displayScale);
            p.size.spacing = event.style->getSizeRole(SizeRole::SpacingSmall, event.displayScale);
            p.size.border = event.style->getSizeRole(SizeRole::Border, event.displayScale);

            p.size.fontMetrics = event.getFontMetrics(p.fontRole);
            auto fontInfo = event.style->getFontRole(p.fontRole, event.displayScale);
            p.size.fontInfo = fontInfo;
            p.size.textSize = math::Vector2i();
            for (const auto& i : p.items)
            {
                if (!i.text.empty())
                {
                    const math::Vector2i textSize = event.fontSystem->getSize(i.text, fontInfo);
                    p.size.textSize.x = std::max(p.size.textSize.x, textSize.x);
                    p.size.textSize.y = std::max(p.size.textSize.y, textSize.y);
                }
            }

            _sizeHint.x = p.size.textSize.x + p.size.margin * 2;
            _sizeHint.y = p.size.fontMetrics.lineHeight;
            if (p.iconImage)
            {
                _sizeHint.x += p.iconImage->getWidth();
                if (!p.text.empty())
                {
                    _sizeHint.x += p.size.spacing;
                }
                _sizeHint.y = std::max(
                    _sizeHint.y,
                    static_cast<int>(p.iconImage->getHeight()));
            }
            if (p.arrowIconImage)
            {
                _sizeHint.x += p.arrowIconImage->getWidth();
                _sizeHint.x += p.size.spacing;
                _sizeHint.y = std::max(
                    _sizeHint.y,
                    static_cast<int>(p.arrowIconImage->getHeight()));
            }
            _sizeHint.x +=
                p.size.margin * 2 +
                p.size.border * 4;
            _sizeHint.y +=
                p.size.margin * 2 +
                p.size.border * 4;
        }

        void ComboBox::clipEvent(
            const math::BBox2i& clipRect,
            bool clipped,
            const ClipEvent& event)
        {
            const bool changed = clipped != _clipped;
            IWidget::clipEvent(clipRect, clipped, event);
            if (changed && clipped)
            {
                _resetMouse();
            }
        }

        void ComboBox::drawEvent(
            const math::BBox2i& drawRect,
            const DrawEvent& event)
        {
            IWidget::drawEvent(drawRect, event);
            TLRENDER_P();

            const math::BBox2i& g = _geometry;
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

            const math::BBox2i g2 = g.margin(-p.size.border * 2);
            event.render->drawRect(
                g2,
                event.style->getColorRole(ColorRole::Button));

            if (p.mouse.pressed && _geometry.contains(p.mouse.cursorPos))
            {
                event.render->drawRect(
                    g2,
                    event.style->getColorRole(ColorRole::Pressed));
            }
            else if (p.mouse.inside)
            {
                event.render->drawRect(
                    g2,
                    event.style->getColorRole(ColorRole::Hover));
            }

            const math::BBox2i g3 = g2.margin(-p.size.margin);
            int x = g3.x();
            if (p.iconImage)
            {
                const imaging::Size& iconSize = p.iconImage->getSize();
                event.render->drawImage(
                    p.iconImage,
                    math::BBox2i(
                        x,
                        g3.y() + g3.h() / 2 - iconSize.h / 2,
                        iconSize.w,
                        iconSize.h),
                    event.style->getColorRole(enabled ?
                        ColorRole::Text :
                        ColorRole::TextDisabled));
                x += iconSize.w + p.size.spacing;
            }

            if (!p.text.empty())
            {
                if (p.draw.glyphs.empty())
                {
                    p.draw.glyphs = event.fontSystem->getGlyphs(p.text, p.size.fontInfo);
                }
                const math::Vector2i pos(
                    x + p.size.margin,
                    g3.y() + g3.h() / 2 - p.size.textSize.y / 2 +
                    p.size.fontMetrics.ascender);
                event.render->drawText(
                    p.draw.glyphs,
                    pos,
                    event.style->getColorRole(enabled ?
                        ColorRole::Text :
                        ColorRole::TextDisabled));
                x += p.size.textSize.x + p.size.margin * 2 + p.size.spacing;
            }

            if (p.arrowIconImage)
            {
                const imaging::Size& iconSize = p.arrowIconImage->getSize();
                event.render->drawImage(
                    p.arrowIconImage,
                    math::BBox2i(
                        x,
                        g3.y() + g3.h() / 2 - iconSize.h / 2,
                        iconSize.w,
                        iconSize.h),
                    event.style->getColorRole(enabled ?
                        ColorRole::Text :
                        ColorRole::TextDisabled));
            }
        }

        void ComboBox::mouseEnterEvent()
        {
            TLRENDER_P();
            p.mouse.inside = true;
            _updates |= Update::Draw;
        }

        void ComboBox::mouseLeaveEvent()
        {
            TLRENDER_P();
            p.mouse.inside = false;
            _updates |= Update::Draw;
        }

        void ComboBox::mouseMoveEvent(MouseMoveEvent& event)
        {
            TLRENDER_P();
            event.accept = true;
            p.mouse.cursorPos = event.pos;
        }

        void ComboBox::mousePressEvent(MouseClickEvent& event)
        {
            TLRENDER_P();
            event.accept = true;
            p.mouse.pressed = true;
            _updates |= Update::Draw;
            _click();
        }

        void ComboBox::mouseReleaseEvent(MouseClickEvent& event)
        {
            TLRENDER_P();
            event.accept = true;
            p.mouse.pressed = false;
            _updates |= Update::Draw;
        }

        void ComboBox::keyPressEvent(KeyEvent& event)
        {
            TLRENDER_P();
            if (0 == event.modifiers)
            {
                switch (event.key)
                {
                case Key::Up:
                    event.accept = true;
                    _commitIndex(p.currentIndex - 1);
                    break;
                case Key::Down:
                    event.accept = true;
                    _commitIndex(p.currentIndex + 1);
                    break;
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
        }

        void ComboBox::keyReleaseEvent(KeyEvent& event)
        {
            event.accept = true;
        }

        ComboBoxItem ComboBox::_getItem(int value) const
        {
            TLRENDER_P();
            ComboBoxItem out;
            if (value >= 0 && value < static_cast<int>(p.items.size()))
            {
                out = p.items[value];
            }
            return out;
        }

        void ComboBox::_click()
        {
            TLRENDER_P();
            takeKeyFocus();
            if (auto context = _context.lock())
            {
                if (auto eventLoop = getEventLoop().lock())
                {
                    if (!p.menu)
                    {
                        p.menu = ComboBoxMenu::create(p.items, context);
                        p.menu->open(eventLoop, _geometry);
                        auto weak = std::weak_ptr<ComboBox>(std::dynamic_pointer_cast<ComboBox>(shared_from_this()));
                        p.menu->setCallback(
                            [weak](int index)
                            {
                                if (auto widget = weak.lock())
                                {
                                    widget->_p->menu->close();
                                    widget->_p->menu.reset();
                                    widget->takeKeyFocus();
                                    if (index != -1)
                                    {
                                        widget->_commitIndex(index);
                                    }
                                }
                            });
                        p.menu->setCloseCallback(
                            [weak]
                            {
                                if (auto widget = weak.lock())
                                {
                                    widget->_p->menu.reset();
                                }
                            });
                    }
                    else
                    {
                        p.menu->close();
                        p.menu.reset();
                    }
                }
            }
        }

        void ComboBox::_resetMouse()
        {
            TLRENDER_P();
            if (p.mouse.pressed || p.mouse.inside)
            {
                p.mouse.pressed = false;
                p.mouse.inside = false;
                _updates |= Update::Draw;
            }
        }

        void ComboBox::_commitIndex(int value)
        {
            TLRENDER_P();
            const int currentIndex = p.currentIndex;
            setCurrentIndex(value);
            if (p.currentIndex != currentIndex)
            {
                if (p.indexCallback)
                {
                    p.indexCallback(p.currentIndex);
                }
                if (p.itemCallback)
                {
                    p.itemCallback(_getItem(p.currentIndex));
                }
            }
        }
    }
}
