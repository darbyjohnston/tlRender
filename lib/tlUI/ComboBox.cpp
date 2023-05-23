// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2023 Darby Johnston
// All rights reserved.

#include <tlUI/ComboBox.h>

#include <tlUI/ButtonGroup.h>
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
            class MenuWidget : public IWidget
            {
                TLRENDER_NON_COPYABLE(MenuWidget);

            protected:
                void _init(
                    const std::shared_ptr<system::Context>&,
                    const std::shared_ptr<IWidget>& parent = nullptr);

                MenuWidget();

            public:
                ~MenuWidget() override;

                static std::shared_ptr<MenuWidget> create(
                    const std::shared_ptr<system::Context>&,
                    const std::shared_ptr<IWidget>& parent = nullptr);

                void setGeometry(const math::BBox2i&) override;
                void sizeHintEvent(const SizeHintEvent&) override;
                void drawEvent(
                    const math::BBox2i&,
                    const DrawEvent&) override;

            private:
                int _border = 0;
            };

            void MenuWidget::_init(
                const std::shared_ptr<system::Context>& context,
                const std::shared_ptr<IWidget>& parent)
            {
                IWidget::_init("tl::ui::MenuWidget", context, parent);
            }

            MenuWidget::MenuWidget()
            {}

            MenuWidget::~MenuWidget()
            {}

            std::shared_ptr<MenuWidget> MenuWidget::create(
                const std::shared_ptr<system::Context>& context,
                const std::shared_ptr<IWidget>& parent)
            {
                auto out = std::shared_ptr<MenuWidget>(new MenuWidget);
                out->_init(context, parent);
                return out;
            }

            void MenuWidget::setGeometry(const math::BBox2i& value)
            {
                IWidget::setGeometry(value);
                const math::BBox2i g = value.margin(-_border);
                math::Vector2i pos = g.min;
                for (const auto& child : _children)
                {
                    const math::Vector2i& sizeHint = child->getSizeHint();
                    child->setGeometry(math::BBox2i(
                        pos.x,
                        pos.y,
                        g.w(),
                        sizeHint.y));
                    pos.y += sizeHint.y;
                }
            }

            void MenuWidget::sizeHintEvent(const SizeHintEvent& event)
            {
                IWidget::sizeHintEvent(event);

                _border = event.style->getSizeRole(SizeRole::Border, event.displayScale);

                _sizeHint = math::Vector2i();
                for (const auto& child : _children)
                {
                    const math::Vector2i& sizeHint = child->getSizeHint();
                    _sizeHint.x = std::max(_sizeHint.x, sizeHint.x);
                    _sizeHint.y += sizeHint.y;
                }
                _sizeHint.x += _border * 2;
                _sizeHint.y += _border * 2;
            }

            void MenuWidget::drawEvent(
                const math::BBox2i& drawRect,
                const DrawEvent& event)
            {
                IWidget::drawEvent(drawRect, event);

                const math::BBox2i& g = _geometry;

                event.render->drawMesh(
                    border(g, _border),
                    math::Vector2i(),
                    event.style->getColorRole(ColorRole::Border));

                const math::BBox2i g2 = g.margin(-_border);
                event.render->drawRect(
                    g2,
                    event.style->getColorRole(ColorRole::Button));
            }

            class Overlay : public IWidget
            {
                TLRENDER_NON_COPYABLE(Overlay);

            protected:
                void _init(
                    const std::vector<ComboBoxItem>&,
                    const math::BBox2i& comboBoxGeometry,
                    const std::shared_ptr<system::Context>&,
                    const std::shared_ptr<IWidget>& parent = nullptr);

                Overlay();

            public:
                ~Overlay() override;

                static std::shared_ptr<Overlay> create(
                    const std::vector<ComboBoxItem>&,
                    const math::BBox2i& comboBoxGeometry,
                    const std::shared_ptr<system::Context>&,
                    const std::shared_ptr<IWidget>& parent = nullptr);

                void setCallback(const std::function<void(int)>&);

                void setGeometry(const math::BBox2i&) override;
                void drawEvent(
                    const math::BBox2i&,
                    const DrawEvent&) override;
                void enterEvent() override;
                void leaveEvent() override;
                void mouseMoveEvent(MouseMoveEvent&) override;
                void mousePressEvent(MouseClickEvent&) override;
                void mouseReleaseEvent(MouseClickEvent&) override;
                void keyPressEvent(KeyEvent&) override;
                void keyReleaseEvent(KeyEvent&) override;

            private:
                math::BBox2i _comboBoxGeometry;
                std::shared_ptr<ButtonGroup> _buttonGroup;
                std::shared_ptr<MenuWidget> _menuWidget;
                std::function<void(int)> _callback;
            };

            void Overlay::_init(
                const std::vector<ComboBoxItem>& items,
                const math::BBox2i& comboBoxGeometry,
                const std::shared_ptr<system::Context>& context,
                const std::shared_ptr<IWidget>& parent)
            {
                IWidget::_init("tl::ui::Overlay", context, parent);

                _comboBoxGeometry = comboBoxGeometry;

                std::vector<std::shared_ptr<ListButton> > buttons;
                _buttonGroup = ButtonGroup::create(ButtonGroupType::Click, context);
                for (const auto& item : items)
                {
                    auto button = ListButton::create(context);
                    button->setText(item.text);
                    button->setIcon(item.icon);
                    buttons.push_back(button);
                    _buttonGroup->addButton(button);
                }

                _menuWidget = MenuWidget::create(context, shared_from_this());
                for (const auto& button : buttons)
                {
                    button->setParent(_menuWidget);
                }

                _buttonGroup->setClickedCallback(
                    [this](int index)
                    {
                        if (_callback)
                        {
                            _callback(index);
                        }
                    });
            }

            Overlay::Overlay()
            {}

            Overlay::~Overlay()
            {}

            std::shared_ptr<Overlay> Overlay::create(
                const std::vector<ComboBoxItem>& items,
                const math::BBox2i& comboBoxGeometry,
                const std::shared_ptr<system::Context>& context,
                const std::shared_ptr<IWidget>& parent)
            {
                auto out = std::shared_ptr<Overlay>(new Overlay);
                out->_init(items, comboBoxGeometry, context, parent);
                return out;
            }

            void Overlay::setCallback(const std::function<void(int)>& value)
            {
                _callback = value;
            }

            void Overlay::setGeometry(const math::BBox2i& value)
            {
                IWidget::setGeometry(value);
                const math::Vector2i& sizeHint = _menuWidget->getSizeHint();
                std::vector<math::BBox2i> bboxes;
                bboxes.push_back(math::BBox2i(
                    _comboBoxGeometry.min.x,
                    _comboBoxGeometry.max.y,
                    sizeHint.x,
                    sizeHint.y));
                bboxes.push_back(math::BBox2i(
                    _comboBoxGeometry.max.x - sizeHint.x + 1,
                    _comboBoxGeometry.max.y,
                    sizeHint.x,
                    sizeHint.y));
                bboxes.push_back(math::BBox2i(
                    _comboBoxGeometry.min.x,
                    _comboBoxGeometry.min.y - sizeHint.y + 1,
                    sizeHint.x,
                    sizeHint.y));
                bboxes.push_back(math::BBox2i(
                    _comboBoxGeometry.max.x - sizeHint.x + 1,
                    _comboBoxGeometry.min.y - sizeHint.y + 1,
                    sizeHint.x,
                    sizeHint.y));
                for (auto& bbox : bboxes)
                {
                    bbox = bbox.intersect(value);
                }
                std::stable_sort(
                    bboxes.begin(),
                    bboxes.end(),
                    [](const math::BBox2i& a, const math::BBox2i& b)
                    {
                        return a.getArea() > b.getArea();
                    });
                _menuWidget->setGeometry(bboxes.front());
            }

            void Overlay::drawEvent(
                const math::BBox2i& drawRect,
                const DrawEvent& event)
            {
                IWidget::drawEvent(drawRect, event);
                //event.render->drawRect(
                //    _geometry,
                //    imaging::Color4f(0.F, 0.F, 0.F, .2F));
            }

            void Overlay::enterEvent()
            {}

            void Overlay::leaveEvent()
            {}

            void Overlay::mouseMoveEvent(MouseMoveEvent& event)
            {
                event.accept = true;
            }

            void Overlay::mousePressEvent(MouseClickEvent& event)
            {
                event.accept = true;
                if (_callback)
                {
                    _callback(-1);
                }
            }

            void Overlay::mouseReleaseEvent(MouseClickEvent& event)
            {
                event.accept = true;
            }

            void Overlay::keyPressEvent(KeyEvent& event)
            {
                switch (event.key)
                {
                case Key::Tab:
                    break;
                case Key::Escape:
                    event.accept = true;
                    if (_callback)
                    {
                        _callback(-1);
                    }
                    break;
                default:
                    event.accept = true;
                    break;
                }
            }

            void Overlay::keyReleaseEvent(KeyEvent& event)
            {
                event.accept = true;
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
            bool icon2Init = false;
            std::future<std::shared_ptr<imaging::Image> > icon2Future;
            std::shared_ptr<imaging::Image> icon2Image;

            struct SizeData
            {
                int margin = 0;
                int spacing = 0;
                int border = 0;
                imaging::FontInfo fontInfo = imaging::FontInfo("", 0);
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

            std::shared_ptr<Overlay> overlay;
        };

        void ComboBox::_init(
            const std::shared_ptr<system::Context>& context,
            const std::shared_ptr<IWidget>& parent)
        {
            IWidget::_init("tl::ui::ComboBox", context, parent);
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

        bool ComboBox::acceptsKeyFocus() const
        {
            return true;
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
                p.icon2Init = true;
                p.icon2Future = std::future<std::shared_ptr<imaging::Image> >();
                p.icon2Image.reset();
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
            if (p.icon2Init)
            {
                p.icon2Init = false;
                p.icon2Future = event.iconLibrary->request("ComboBoxArrow", event.displayScale);
            }
            if (p.icon2Future.valid() &&
                p.icon2Future.wait_for(std::chrono::seconds(0)) == std::future_status::ready)
            {
                p.icon2Image = p.icon2Future.get();
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
            if (fontInfo != p.size.fontInfo)
            {
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
            if (p.icon2Image)
            {
                _sizeHint.x += p.icon2Image->getWidth();
                _sizeHint.x += p.size.spacing;
                _sizeHint.y = std::max(
                    _sizeHint.y,
                    static_cast<int>(p.icon2Image->getHeight()));
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

            int x = g2.x() + p.size.margin;
            if (p.iconImage)
            {
                const imaging::Size& iconSize = p.iconImage->getSize();
                event.render->drawImage(
                    p.iconImage,
                    math::BBox2i(
                        x,
                        g2.y() + g2.h() / 2 - iconSize.h / 2,
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
                    g2.y() + g2.h() / 2 - p.size.textSize.y / 2 +
                    p.size.fontMetrics.ascender);
                event.render->drawText(
                    p.draw.glyphs,
                    pos,
                    event.style->getColorRole(enabled ?
                        ColorRole::Text :
                        ColorRole::TextDisabled));
                x += p.size.textSize.x + p.size.margin * 2 + p.size.spacing;
            }

            if (p.icon2Image)
            {
                const imaging::Size& iconSize = p.icon2Image->getSize();
                event.render->drawImage(
                    p.icon2Image,
                    math::BBox2i(
                        x,
                        g2.y() + g2.h() / 2 - iconSize.h / 2,
                        iconSize.w,
                        iconSize.h),
                    event.style->getColorRole(enabled ?
                        ColorRole::Text :
                        ColorRole::TextDisabled));
            }
        }

        void ComboBox::enterEvent()
        {
            TLRENDER_P();
            p.mouse.inside = true;
            _updates |= Update::Draw;
        }

        void ComboBox::leaveEvent()
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
            _resetMouse();
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
            switch (event.key)
            {
            case Key::Up:
                _commitIndex(p.currentIndex - 1);
                break;
            case Key::Down:
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
            if (auto context = _context.lock())
            {
                if (auto eventLoop = getEventLoop().lock())
                {
                    p.overlay = Overlay::create(
                        p.items,
                        _geometry.margin(-p.size.border),
                        context);
                    auto weak = std::weak_ptr<ComboBox>(std::dynamic_pointer_cast<ComboBox>(shared_from_this()));
                    p.overlay->setCallback(
                        [weak](int index)
                        {
                            if (auto widget = weak.lock())
                            {
                                widget->_p->overlay.reset();
                                if (widget->acceptsKeyFocus())
                                {
                                    widget->takeKeyFocus();
                                }
                                widget->_updates |= Update::Size;
                                widget->_updates |= Update::Draw;
                                if (index != -1)
                                {
                                    widget->_commitIndex(index);
                                }
                            }
                        });
                    eventLoop->addWidget(p.overlay);
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
