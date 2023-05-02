// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2023 Darby Johnston
// All rights reserved.

#include "ScrollAreas.h"

#include <tlUI/GridLayout.h>
#include <tlUI/RowLayout.h>
#include <tlUI/ScrollWidget.h>
#include <tlUI/ToolButton.h>

#include <tlCore/StringFormat.h>

namespace tl
{
    namespace examples
    {
        namespace ui_glfw
        {
            struct ScrollAreas::Private
            {
                std::shared_ptr<ui::RowLayout> layout;
            };

            void ScrollAreas::_init(
                const std::shared_ptr<system::Context>& context)
            {
                IWidget::_init("ScrollAreas", context);
                TLRENDER_P();

                auto hLayout = ui::HorizontalLayout::create(context);
                hLayout->setSpacingRole(ui::SizeRole::None);
                for (size_t i = 0; i < 20; ++i)
                {
                    auto toolButton = ui::ToolButton::create(context, hLayout);
                    toolButton->setText(string::Format("Button {0}").arg(i));
                }
                auto scrollWidget0 = ui::ScrollWidget::create(context, ui::ScrollType::Horizontal);
                scrollWidget0->setWidget(hLayout);

                auto vLayout = ui::VerticalLayout::create(context);
                vLayout->setSpacingRole(ui::SizeRole::None);
                for (size_t i = 0; i < 20; ++i)
                {
                    auto toolButton = ui::ToolButton::create(context, vLayout);
                    toolButton->setText(string::Format("Button {0}").arg(i));
                }
                auto scrollWidget1 = ui::ScrollWidget::create(context, ui::ScrollType::Vertical);
                scrollWidget1->setWidget(vLayout);

                auto gridLayout = ui::GridLayout::create(context);
                gridLayout->setSpacingRole(ui::SizeRole::None);
                for (size_t j = 0; j < 20; ++j)
                {
                    for (size_t i = 0; i < 20; ++i)
                    {
                        auto toolButton = ui::ToolButton::create(context, gridLayout);
                        toolButton->setText(string::Format("Button {0},{1}").arg(j).arg(i));
                        gridLayout->setGridPos(toolButton, j, i);
                    }
                }
                auto scrollWidget2 = ui::ScrollWidget::create(context);
                scrollWidget2->setWidget(gridLayout);

                p.layout = ui::VerticalLayout::create(context, shared_from_this());
                scrollWidget0->setParent(p.layout);
                hLayout = ui::HorizontalLayout::create(context, p.layout);
                hLayout->setVStretch(ui::Stretch::Expanding);
                scrollWidget1->setParent(hLayout);
                scrollWidget2->setParent(hLayout);
            }

            ScrollAreas::ScrollAreas() :
                _p(new Private)
            {}

            ScrollAreas::~ScrollAreas()
            {}

            std::shared_ptr<ScrollAreas> ScrollAreas::create(
                const std::shared_ptr<system::Context>& context)
            {
                auto out = std::shared_ptr<ScrollAreas>(new ScrollAreas);
                out->_init(context);
                return out;
            }

            void ScrollAreas::setGeometry(const math::BBox2i& value)
            {
                IWidget::setGeometry(value);
                _p->layout->setGeometry(value);
            }
        }
    }
}
