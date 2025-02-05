// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2025 Darby Johnston
// All rights reserved.

#include <tlUI/MDIWidget.h>

#include <tlUI/Divider.h>
#include <tlUI/DrawUtil.h>
#include <tlUI/Label.h>
#include <tlUI/RowLayout.h>
#include <tlUI/ToolButton.h>

#include <dtk/core/Error.h>
#include <dtk/core/String.h>

namespace tl
{
    namespace ui
    {
        DTK_ENUM_IMPL(
            MDIResize,
            "North",
            "NorthEast",
            "East",
            "SouthEast",
            "South",
            "SouthWest",
            "West",
            "NorthWest");

        struct MDIWidget::Private
        {
            std::shared_ptr<Label> titleLabel;
            std::shared_ptr<ToolButton> closeButton;
            std::shared_ptr<IWidget> widget;
            std::shared_ptr<VerticalLayout> widgetLayout;
            std::shared_ptr<VerticalLayout> layout;
            std::function<void(bool)> pressCallback;
            std::function<void(const dtk::V2I&)> moveCallback;
            std::function<void(MDIResize, const dtk::V2I&)> resizeCallback;

            struct SizeData
            {
                bool sizeInit = true;
                int border = 0;
                int handle = 0;
                int shadow = 0;

                dtk::Box2I insideGeometry;
            };
            SizeData size;

            struct MouseData
            {
                MDIResize resize = MDIResize::None;
                std::map<MDIResize, dtk::Box2I> resizeBoxes;
            };
            MouseData mouse;
        };

        void MDIWidget::_init(
            const std::string& title,
            const std::shared_ptr<dtk::Context>& context,
            const std::shared_ptr<IWidget>& parent)
        {
            IWidget::_init("tl::ui::MDIWidget", context, parent);
            DTK_P();

            _setMouseHover(true);
            _setMousePress(true);

            p.titleLabel = Label::create(context);
            p.titleLabel->setText(title);
            p.titleLabel->setMarginRole(SizeRole::MarginInside);
            p.titleLabel->setHStretch(Stretch::Expanding);

            p.closeButton = ToolButton::create(context);
            p.closeButton->setIcon("CloseSmall");
            p.closeButton->setToolTip("Close the window");

            p.layout = VerticalLayout::create(context, shared_from_this());
            p.layout->setSpacingRole(SizeRole::None);
            auto hLayout = HorizontalLayout::create(context, p.layout);
            hLayout->setSpacingRole(SizeRole::None);
            hLayout->setBackgroundRole(ColorRole::Button);
            p.titleLabel->setParent(hLayout);
            p.closeButton->setParent(hLayout);
            Divider::create(Orientation::Vertical, context, p.layout);
            p.widgetLayout = VerticalLayout::create(context, p.layout);
            p.widgetLayout->setMarginRole(SizeRole::MarginInside);
            p.widgetLayout->setVStretch(Stretch::Expanding);

            p.closeButton->setClickedCallback(
                [this]
                {
                    setParent(nullptr);
                });
        }

        MDIWidget::MDIWidget() :
            _p(new Private)
        {}

        MDIWidget::~MDIWidget()
        {}

        std::shared_ptr<MDIWidget> MDIWidget::create(
            const std::string& title,
            const std::shared_ptr<dtk::Context>& context,
            const std::shared_ptr<IWidget>& parent)
        {
            auto out = std::shared_ptr<MDIWidget>(new MDIWidget);
            out->_init(title, context, parent);
            return out;
        }

        void MDIWidget::setTitle(const std::string& value)
        {
            _p->titleLabel->setText(value);
        }

        void MDIWidget::setWidget(const std::shared_ptr<IWidget>& value)
        {
            DTK_P();
            if (p.widget)
            {
                p.widget->setParent(nullptr);
            }
            p.widget = value;
            if (p.widget)
            {
                p.widget->setParent(p.widgetLayout);
            }
            _updates |= Update::Size;
            _updates |= Update::Draw;
        }

        void MDIWidget::setPressCallback(const std::function<void(bool)>& value)
        {
            _p->pressCallback = value;
        }

        void MDIWidget::setMoveCallback(const std::function<void(const dtk::V2I&)>& value)
        {
            _p->moveCallback = value;
        }

        void MDIWidget::setResizeCallback(const std::function<void(MDIResize, const dtk::V2I&)>& value)
        {
            _p->resizeCallback = value;
        }

        const dtk::Box2I& MDIWidget::getInsideGeometry() const
        {
            return _p->size.insideGeometry;
        }

        void MDIWidget::setGeometry(const dtk::Box2I& value)
        {
            IWidget::setGeometry(value);
            DTK_P();
            const int margin = std::max(p.size.handle, p.size.shadow);
            const dtk::Box2I g = dtk::margin(
                value,
                -(margin + p.size.border),
                -(p.size.handle + p.size.border),
                -(margin + p.size.border),
                -(margin + p.size.border));
            p.size.insideGeometry = g;
            p.layout->setGeometry(g);
            p.mouse.resizeBoxes.clear();
            p.mouse.resizeBoxes[MDIResize::North] = dtk::Box2I(
                g.min.x + p.size.handle,
                g.min.y - p.size.handle,
                g.w() - p.size.handle * 2,
                p.size.handle);
            p.mouse.resizeBoxes[MDIResize::NorthEast] = dtk::Box2I(
                g.max.x - p.size.handle,
                g.min.y - p.size.handle,
                p.size.handle * 2,
                p.size.handle * 2);
            p.mouse.resizeBoxes[MDIResize::East] = dtk::Box2I(
                g.max.x,
                g.min.y + p.size.handle,
                p.size.handle,
                g.h() - p.size.handle * 2);
            p.mouse.resizeBoxes[MDIResize::SouthEast] = dtk::Box2I(
                g.max.x - p.size.handle,
                g.max.y - p.size.handle,
                p.size.handle * 2,
                p.size.handle * 2);
            p.mouse.resizeBoxes[MDIResize::South] = dtk::Box2I(
                g.min.x + p.size.handle,
                g.max.y,
                g.w() - p.size.handle * 2,
                p.size.handle);
            p.mouse.resizeBoxes[MDIResize::SouthWest] = dtk::Box2I(
                g.min.x - p.size.handle,
                g.max.y - p.size.handle,
                p.size.handle * 2,
                p.size.handle * 2);
            p.mouse.resizeBoxes[MDIResize::West] = dtk::Box2I(
                g.min.x - p.size.handle,
                g.min.y + p.size.handle,
                p.size.handle,
                g.h() - p.size.handle * 2);
            p.mouse.resizeBoxes[MDIResize::NorthWest] = dtk::Box2I(
                g.min.x - p.size.handle,
                g.min.y - p.size.handle,
                p.size.handle * 2,
                p.size.handle * 2);
        }

        void MDIWidget::sizeHintEvent(const SizeHintEvent& event)
        {
            const bool displayScaleChanged = event.displayScale != _displayScale;
            IWidget::sizeHintEvent(event);
            DTK_P();

            if (displayScaleChanged || p.size.sizeInit)
            {
                p.size.border = event.style->getSizeRole(SizeRole::Border, _displayScale);
                p.size.handle = event.style->getSizeRole(SizeRole::Handle, _displayScale);
                p.size.shadow = event.style->getSizeRole(SizeRole::Shadow, _displayScale);
            }
            p.size.sizeInit = false;

            const int margin = std::max(p.size.handle, p.size.shadow);
            _sizeHint = p.layout->getSizeHint() + p.size.border * 2;
            _sizeHint.w += margin * 2;
            _sizeHint.h += p.size.handle + margin;
        }

        void MDIWidget::drawEvent(
            const dtk::Box2I& drawRect,
            const DrawEvent& event)
        {
            IWidget::drawEvent(drawRect, event);
            DTK_P();
            const dtk::Box2I& g = _geometry;
            const int margin = std::max(p.size.handle, p.size.shadow);
            const dtk::Box2I g2 = dtk::margin(g, -margin, -p.size.handle, -margin, -margin);
            event.render->drawColorMesh(
                shadow(dtk::margin(g2, p.size.shadow, 0, p.size.shadow, p.size.shadow), p.size.shadow),
                dtk::Color4F(1.F, 1.F, 1.F));
            if (p.mouse.resize != MDIResize::None)
            {
                const auto i = p.mouse.resizeBoxes.find(p.mouse.resize);
                if (i != p.mouse.resizeBoxes.end())
                {
                    event.render->drawRect(
                        i->second,
                        event.style->getColorRole(ColorRole::Checked));
                }
            }
            event.render->drawMesh(
                border(g2, p.size.border),
                event.style->getColorRole(ColorRole::Border));
            const dtk::Box2I g3 = dtk::margin(g2, -p.size.border);
            event.render->drawRect(
                g3,
                event.style->getColorRole(ColorRole::Window));
        }

        void MDIWidget::mouseLeaveEvent()
        {
            IWidget::mouseLeaveEvent();
            DTK_P();
            if (p.mouse.resize != MDIResize::None)
            {
                p.mouse.resize = MDIResize::None;
                _updates |= Update::Draw;
            }
        }

        void MDIWidget::mouseMoveEvent(MouseMoveEvent& event)
        {
            IWidget::mouseMoveEvent(event);
            DTK_P();
            if (!_mouse.press)
            {
                MDIResize resize = MDIResize::None;
                for (const auto& box : p.mouse.resizeBoxes)
                {
                    if (dtk::contains(box.second, event.pos))
                    {
                        resize = box.first;
                        break;
                    }
                }
                if (resize != p.mouse.resize)
                {
                    p.mouse.resize = resize;
                    _updates |= Update::Draw;
                }
            }
            else
            {
                if (p.mouse.resize != MDIResize::None)
                {
                    if (p.resizeCallback)
                    {
                        p.resizeCallback(p.mouse.resize, event.pos - _mouse.pressPos);
                    }
                }
                else if (p.moveCallback)
                {
                    p.moveCallback(event.pos - _mouse.pressPos);
                }
            }
        }

        void MDIWidget::mousePressEvent(MouseClickEvent& event)
        {
            IWidget::mousePressEvent(event);
            DTK_P();
            if (p.pressCallback)
            {
                p.pressCallback(true);
            }
        }

        void MDIWidget::mouseReleaseEvent(MouseClickEvent& event)
        {
            IWidget::mouseReleaseEvent(event);
            DTK_P();
            if (p.pressCallback)
            {
                p.pressCallback(false);
            }
        }
    }
}
