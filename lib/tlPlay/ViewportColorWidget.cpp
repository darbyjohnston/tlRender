// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2025 Darby Johnston
// All rights reserved.

#include <tlPlay/ViewportPrivate.h>

#include <tlUI/ColorSwatch.h>
#include <tlUI/DrawUtil.h>
#include <tlUI/Label.h>
#include <tlUI/RowLayout.h>
#include <tlUI/ToolButton.h>

#include <tlCore/StringFormat.h>

namespace tl
{
    namespace play
    {
        struct ViewportColorWidget::Private
        {
            image::Color4f color;

            std::shared_ptr<ui::ToolButton> closeButton;
            std::shared_ptr<ui::ColorSwatch> swatch;
            std::shared_ptr<ui::Label> label;
            std::shared_ptr<ui::HorizontalLayout> layout;

            struct SizeData
            {
                int margin = 0;
                int border = 0;
            };
            SizeData size;
        };

        void ViewportColorWidget::_init(
            const std::shared_ptr<system::Context>& context,
            const std::shared_ptr<IWidget>& parent)
        {
            IWidget::_init("tl::play::ViewportColorWidget", context, parent);
            TLRENDER_P();

            setBackgroundRole(ui::ColorRole::Window);

            p.closeButton = ui::ToolButton::create(context);
            p.closeButton->setIcon("CloseSmall");

            p.swatch = ui::ColorSwatch::create(context);

            p.label = ui::Label::create(context, shared_from_this());
            p.label->setFontRole(ui::FontRole::Mono);

            p.layout = ui::HorizontalLayout::create(context, shared_from_this());
            p.layout->setSpacingRole(ui::SizeRole::None);
            auto hLayout = ui::HorizontalLayout::create(context, p.layout);
            hLayout->setMarginRole(ui::SizeRole::MarginInside);
            hLayout->setSpacingRole(ui::SizeRole::SpacingTool);
            p.swatch->setParent(hLayout);
            p.label->setParent(hLayout);
            auto vLayout = ui::VerticalLayout::create(context, p.layout);
            vLayout->setSpacingRole(ui::SizeRole::None);
            p.closeButton->setParent(vLayout);

            _colorUpdate();

            auto weak = std::weak_ptr<ViewportColorWidget>(
                std::dynamic_pointer_cast<ViewportColorWidget>(shared_from_this()));
            p.closeButton->setClickedCallback(
                [weak]
                {
                    if (auto widget = weak.lock())
                    {
                        widget->setParent(nullptr);
                    }
                });
        }

        ViewportColorWidget::ViewportColorWidget() :
            _p(new Private)
        {}

        ViewportColorWidget::~ViewportColorWidget()
        {}

        std::shared_ptr<ViewportColorWidget> ViewportColorWidget::create(
            const std::shared_ptr<system::Context>& context,
            const std::shared_ptr<IWidget>& parent)
        {
            auto out = std::shared_ptr<ViewportColorWidget>(new ViewportColorWidget);
            out->_init(context, parent);
            return out;
        }

        void ViewportColorWidget::setColor(const image::Color4f& value)
        {
            TLRENDER_P();
            if (value == p.color)
                return;
            p.color = value;
            _colorUpdate();
        }

        void ViewportColorWidget::setGeometry(const math::Box2i& value)
        {
            IWidget::setGeometry(value);
            TLRENDER_P();
            p.layout->setGeometry(value.margin(-p.size.border));
        }

        void ViewportColorWidget::sizeHintEvent(const ui::SizeHintEvent& event)
        {
            IWidget::sizeHintEvent(event);
            TLRENDER_P();
            p.size.margin = event.style->getSizeRole(ui::SizeRole::MarginSmall, event.displayScale);
            p.size.border = event.style->getSizeRole(ui::SizeRole::Border, event.displayScale);
            _sizeHint = p.layout->getSizeHint() + p.size.border * 2;
        }

        void ViewportColorWidget::drawEvent(const math::Box2i& drawRect, const ui::DrawEvent& event)
        {
            IWidget::drawEvent(drawRect, event);
            TLRENDER_P();
            
            const math::Box2i& g = getGeometry();
            event.render->drawMesh(
                ui::border(g, p.size.border),
                math::Vector2i(),
                event.style->getColorRole(ui::ColorRole::Border));

            const math::Box2i g2 = g.margin(-p.size.border);
            geom::TriangleMesh2 mesh;
            mesh.v.push_back(math::Vector2f(g2.min.x, g2.min.y));
            mesh.v.push_back(math::Vector2f(g2.min.x + p.size.margin, g2.min.y));
            mesh.v.push_back(math::Vector2f(g2.min.x, g2.min.y + p.size.margin));
            mesh.triangles.push_back({ 1, 2, 3 });
            event.render->drawMesh(
                mesh,
                math::Vector2i(),
                event.style->getColorRole(ui::ColorRole::Text));
        }

        void ViewportColorWidget::_colorUpdate()
        {
            TLRENDER_P();
            p.swatch->setColor(p.color);
            p.label->setText(
                string::Format(
                    "R:{0}\n"
                    "G:{1}\n"
                    "B:{2}\n"
                    "A:{3}").
                arg(p.color.r, 2).
                arg(p.color.g, 2).
                arg(p.color.b, 2).
                arg(p.color.a, 2));
        }
    }
}
