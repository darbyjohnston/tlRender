// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2025 Darby Johnston
// All rights reserved.

#include <tlPlay/ViewportPrivate.h>

#include <dtk/ui/ColorSwatch.h>
#include <dtk/ui/DrawUtil.h>
#include <dtk/ui/Label.h>
#include <dtk/ui/RowLayout.h>
#include <dtk/ui/ToolButton.h>
#include <dtk/core/Format.h>

namespace tl
{
    namespace play
    {
        struct ViewportColorWidget::Private
        {
            dtk::Color4F color;

            std::shared_ptr<dtk::ToolButton> closeButton;
            std::shared_ptr<dtk::ColorSwatch> swatch;
            std::shared_ptr<dtk::Label> label;
            std::shared_ptr<dtk::HorizontalLayout> layout;

            struct SizeData
            {
                int margin = 0;
                int border = 0;
            };
            SizeData size;
        };

        void ViewportColorWidget::_init(
            const std::shared_ptr<dtk::Context>& context,
            const std::shared_ptr<IWidget>& parent)
        {
            IWidget::_init(context, "tl::play::ViewportColorWidget", parent);
            DTK_P();

            setBackgroundRole(dtk::ColorRole::Window);

            p.closeButton = dtk::ToolButton::create(context);
            p.closeButton->setIcon("CloseSmall");

            p.swatch = dtk::ColorSwatch::create(context);

            p.label = dtk::Label::create(context, shared_from_this());
            p.label->setFontRole(dtk::FontRole::Mono);

            p.layout = dtk::HorizontalLayout::create(context, shared_from_this());
            p.layout->setSpacingRole(dtk::SizeRole::None);
            auto hLayout = dtk::HorizontalLayout::create(context, p.layout);
            hLayout->setMarginRole(dtk::SizeRole::MarginInside);
            hLayout->setSpacingRole(dtk::SizeRole::SpacingTool);
            p.swatch->setParent(hLayout);
            p.label->setParent(hLayout);
            auto vLayout = dtk::VerticalLayout::create(context, p.layout);
            vLayout->setSpacingRole(dtk::SizeRole::None);
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
            const std::shared_ptr<dtk::Context>& context,
            const std::shared_ptr<IWidget>& parent)
        {
            auto out = std::shared_ptr<ViewportColorWidget>(new ViewportColorWidget);
            out->_init(context, parent);
            return out;
        }

        void ViewportColorWidget::setColor(const dtk::Color4F& value)
        {
            DTK_P();
            if (value == p.color)
                return;
            p.color = value;
            _colorUpdate();
        }

        void ViewportColorWidget::setGeometry(const dtk::Box2I& value)
        {
            IWidget::setGeometry(value);
            DTK_P();
            p.layout->setGeometry(dtk::margin(value, -p.size.border));
        }

        void ViewportColorWidget::sizeHintEvent(const dtk::SizeHintEvent& event)
        {
            IWidget::sizeHintEvent(event);
            DTK_P();
            p.size.margin = event.style->getSizeRole(dtk::SizeRole::MarginSmall, event.displayScale);
            p.size.border = event.style->getSizeRole(dtk::SizeRole::Border, event.displayScale);
            _setSizeHint(p.layout->getSizeHint() + p.size.border * 2);
        }

        void ViewportColorWidget::drawEvent(const dtk::Box2I& drawRect, const dtk::DrawEvent& event)
        {
            IWidget::drawEvent(drawRect, event);
            DTK_P();
            
            const dtk::Box2I& g = getGeometry();
            event.render->drawMesh(
                dtk::border(g, p.size.border),
                event.style->getColorRole(dtk::ColorRole::Border));

            const dtk::Box2I g2 = dtk::margin(g, -p.size.border);
            dtk::TriMesh2F mesh;
            mesh.v.push_back(dtk::V2F(g2.min.x, g2.min.y));
            mesh.v.push_back(dtk::V2F(g2.min.x + p.size.margin, g2.min.y));
            mesh.v.push_back(dtk::V2F(g2.min.x, g2.min.y + p.size.margin));
            mesh.triangles.push_back({ 1, 2, 3 });
            event.render->drawMesh(
                mesh,
                event.style->getColorRole(dtk::ColorRole::Text));
        }

        void ViewportColorWidget::_colorUpdate()
        {
            DTK_P();
            p.swatch->setColor(p.color);
            p.label->setText(
                dtk::Format(
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
