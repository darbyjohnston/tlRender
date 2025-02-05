// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2025 Darby Johnston
// All rights reserved.

#include <tlUI/ComboBox.h>

#include <tlUI/ButtonGroup.h>
#include <tlUI/DrawUtil.h>
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
                DTK_NON_COPYABLE(ComboBoxMenu);

            protected:
                void _init(
                    const std::vector<ComboBoxItem>&,
                    const std::shared_ptr<dtk::Context>&,
                    const std::shared_ptr<IWidget>& parent);

                ComboBoxMenu();

            public:
                virtual ~ComboBoxMenu();

                static std::shared_ptr<ComboBoxMenu> create(
                    const std::vector<ComboBoxItem>&,
                    const std::shared_ptr<dtk::Context>&,
                    const std::shared_ptr<IWidget>& parent = nullptr);

                void setCallback(const std::function<void(int)>&);

            private:
                std::shared_ptr<ButtonGroup> _buttonGroup;
                std::shared_ptr<VerticalLayout> _layout;
                std::function<void(int)> _callback;
            };

            void ComboBoxMenu::_init(
                const std::vector<ComboBoxItem>& items,
                const std::shared_ptr<dtk::Context>& context,
                const std::shared_ptr<IWidget>& parent)
            {
                IMenuPopup::_init("tl::ui::ComboBoxMenu", context, parent);

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

                _layout = VerticalLayout::create(context);
                _layout->setSpacingRole(SizeRole::None);
                for (const auto& button : buttons)
                {
                    button->setParent(_layout);
                }
                setWidget(_layout);
                
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
                const std::shared_ptr<dtk::Context>& context,
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
            std::future<std::shared_ptr<dtk::Image> > iconFuture;
            std::shared_ptr<dtk::Image> iconImage;
            bool arrowIconInit = false;
            std::future<std::shared_ptr<dtk::Image> > arrowIconFuture;
            std::shared_ptr<dtk::Image> arrowIconImage;

            std::shared_ptr<ComboBoxMenu> menu;

            struct SizeData
            {
                bool sizeInit = true;
                int margin = 0;
                int spacing = 0;
                int border = 0;

                bool textInit = true;
                dtk::FontInfo fontInfo;
                dtk::FontMetrics fontMetrics;
                dtk::Size2I textSize;
            };
            SizeData size;

            struct DrawData
            {
                std::vector<std::shared_ptr<dtk::Glyph> > glyphs;
            };
            DrawData draw;
        };

        void ComboBox::_init(
            const std::shared_ptr<dtk::Context>& context,
            const std::shared_ptr<IWidget>& parent)
        {
            IWidget::_init("tl::ui::ComboBox", context, parent);
            setAcceptsKeyFocus(true);
            _setMouseHover(true);
            _setMousePress(true);
        }

        ComboBox::ComboBox() :
            _p(new Private)
        {}

        ComboBox::~ComboBox()
        {}

        std::shared_ptr<ComboBox> ComboBox::create(
            const std::shared_ptr<dtk::Context>&context,
            const std::shared_ptr<IWidget>&parent)
        {
            auto out = std::shared_ptr<ComboBox>(new ComboBox);
            out->_init(context, parent);
            return out;
        }

        std::shared_ptr<ComboBox> ComboBox::create(
            const std::vector<std::string>& items,
            const std::shared_ptr<dtk::Context>& context,
            const std::shared_ptr<IWidget>& parent)
        {
            auto out = std::shared_ptr<ComboBox>(new ComboBox);
            out->_init(context, parent);
            out->setItems(items);
            return out;
        }

        void ComboBox::setItems(const std::vector<ComboBoxItem>& value)
        {
            DTK_P();
            if (value == p.items)
                return;
            p.items = value;
            p.currentIndex = dtk::clamp(
                p.currentIndex,
                0,
                static_cast<int>(p.items.size()) - 1);
            const ComboBoxItem item = _getItem(p.currentIndex);
            p.text = item.text;
            p.icon = item.icon;
            p.iconInit = true;
            p.iconFuture = std::future<std::shared_ptr<dtk::Image> >();
            p.iconImage.reset();
            p.size.textInit = true;
            _updates |= Update::Size;
            _updates |= Update::Draw;
        }

        void ComboBox::setItems(const std::vector<std::string>& value)
        {
            DTK_P();
            std::vector<ComboBoxItem> items;
            for (const auto& s : value)
            {
                items.push_back({ s, std::string()});
            }
            setItems(items);
        }

        void ComboBox::setCurrentIndex(int value)
        {
            DTK_P();
            const int tmp = dtk::clamp(
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
            p.iconFuture = std::future<std::shared_ptr<dtk::Image> >();
            p.iconImage.reset();
            p.size.textInit = true;
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
            DTK_P();
            if (value == p.fontRole)
                return;
            p.fontRole = value;
            p.size.textInit = true;
            _updates |= Update::Size;
            _updates |= Update::Draw;
        }

        void ComboBox::tickEvent(
            bool parentsVisible,
            bool parentsEnabled,
            const TickEvent& event)
        {
            IWidget::tickEvent(parentsVisible, parentsEnabled, event);
            DTK_P();
            if (_displayScale != p.iconScale)
            {
                p.iconScale = _displayScale;
                p.iconInit = true;
                p.iconFuture = std::future<std::shared_ptr<dtk::Image> >();
                p.iconImage.reset();
                p.arrowIconInit = true;
                p.arrowIconFuture = std::future<std::shared_ptr<dtk::Image> >();
                p.arrowIconImage.reset();
            }
            if (!p.icon.empty() && p.iconInit)
            {
                p.iconInit = false;
                p.iconFuture = event.iconLibrary->request(p.icon, _displayScale);
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
                p.arrowIconFuture = event.iconLibrary->request("MenuArrow", _displayScale);
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
            const bool displayScaleChanged = event.displayScale != _displayScale;
            IWidget::sizeHintEvent(event);
            DTK_P();

            if (displayScaleChanged || p.size.sizeInit)
            {
                p.size.margin = event.style->getSizeRole(SizeRole::MarginInside, _displayScale);
                p.size.spacing = event.style->getSizeRole(SizeRole::SpacingSmall, _displayScale);
                p.size.border = event.style->getSizeRole(SizeRole::Border, _displayScale);
            }
            if (displayScaleChanged || p.size.textInit || p.size.sizeInit)
            {
                p.size.fontInfo = event.style->getFontRole(p.fontRole, _displayScale);
                p.size.fontMetrics = event.fontSystem->getMetrics(p.size.fontInfo);
                p.size.textSize = dtk::Size2I();
                for (const auto& i : p.items)
                {
                    if (!i.text.empty())
                    {
                        const dtk::Size2I textSize = event.fontSystem->getSize(i.text, p.size.fontInfo);
                        p.size.textSize.w = std::max(p.size.textSize.w, textSize.w);
                        p.size.textSize.h = std::max(p.size.textSize.h, textSize.h);
                    }
                }
                p.draw.glyphs.clear();
            }
            p.size.sizeInit = false;
            p.size.textInit = false;

            _sizeHint.w = p.size.textSize.w + p.size.margin * 2;
            _sizeHint.h = p.size.fontMetrics.lineHeight;
            if (p.iconImage)
            {
                _sizeHint.w += p.iconImage->getWidth();
                if (!p.text.empty())
                {
                    _sizeHint.w += p.size.spacing;
                }
                _sizeHint.h = std::max(
                    _sizeHint.h,
                    static_cast<int>(p.iconImage->getHeight()));
            }
            if (p.arrowIconImage)
            {
                _sizeHint.w += p.arrowIconImage->getWidth();
                _sizeHint.w += p.size.spacing;
                _sizeHint.h = std::max(
                    _sizeHint.h,
                    static_cast<int>(p.arrowIconImage->getHeight()));
            }
            _sizeHint.w +=
                p.size.margin * 2 +
                p.size.border * 4;
            _sizeHint.h +=
                p.size.margin * 2 +
                p.size.border * 4;
        }

        void ComboBox::drawEvent(
            const dtk::Box2I& drawRect,
            const DrawEvent& event)
        {
            IWidget::drawEvent(drawRect, event);
            DTK_P();

            const dtk::Box2I& g = _geometry;
            const bool enabled = isEnabled();

            if (_keyFocus)
            {
                event.render->drawMesh(
                    border(g, p.size.border * 2),
                    event.style->getColorRole(ColorRole::KeyFocus));
            }
            else
            {
                event.render->drawMesh(
                    border(dtk::margin(g, -p.size.border), p.size.border),
                    event.style->getColorRole(ColorRole::Border));
            }

            const dtk::Box2I g2 = dtk::margin(g, -p.size.border * 2);
            event.render->drawRect(
                g2,
                event.style->getColorRole(ColorRole::Button));

            if (_mouse.press && dtk::contains(_geometry, _mouse.pos))
            {
                event.render->drawRect(
                    g2,
                    event.style->getColorRole(ColorRole::Pressed));
            }
            else if (_mouse.inside)
            {
                event.render->drawRect(
                    g2,
                    event.style->getColorRole(ColorRole::Hover));
            }

            const dtk::Box2I g3 = dtk::margin(g2, -p.size.margin);
            int x = g3.x();
            if (p.iconImage)
            {
                const dtk::Size2I& iconSize = p.iconImage->getSize();
                event.render->drawImage(
                    p.iconImage,
                    dtk::Box2I(
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
                const dtk::V2I pos(
                    x + p.size.margin,
                    g3.y() + g3.h() / 2 - p.size.textSize.h / 2);
                event.render->drawText(
                    p.draw.glyphs,
                    p.size.fontMetrics,
                    pos,
                    event.style->getColorRole(enabled ?
                        ColorRole::Text :
                        ColorRole::TextDisabled));
            }

            if (p.arrowIconImage)
            {
                const dtk::Size2I& iconSize = p.arrowIconImage->getSize();
                event.render->drawImage(
                    p.arrowIconImage,
                    dtk::Box2I(
                        g3.x() + g3.w() - iconSize.w,
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
            IWidget::mouseEnterEvent();
            _updates |= Update::Draw;
        }

        void ComboBox::mouseLeaveEvent()
        {
            IWidget::mouseLeaveEvent();
            _updates |= Update::Draw;
        }

        void ComboBox::mousePressEvent(MouseClickEvent& event)
        {
            IWidget::mousePressEvent(event);
            DTK_P();
            _click();
            _updates |= Update::Draw;
        }

        void ComboBox::mouseReleaseEvent(MouseClickEvent& event)
        {
            IWidget::mouseReleaseEvent(event);
            _updates |= Update::Draw;
        }

        void ComboBox::keyPressEvent(KeyEvent& event)
        {
            DTK_P();
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
            DTK_P();
            ComboBoxItem out;
            if (value >= 0 && value < static_cast<int>(p.items.size()))
            {
                out = p.items[value];
            }
            return out;
        }

        void ComboBox::_click()
        {
            DTK_P();
            takeKeyFocus();
            if (auto context = _context.lock())
            {
                if (!p.menu)
                {
                    p.menu = ComboBoxMenu::create(p.items, context);
                    p.menu->open(getWindow(), _geometry);
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
                                widget->takeKeyFocus();
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

        void ComboBox::_commitIndex(int value)
        {
            DTK_P();
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
