// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2023 Darby Johnston
// All rights reserved.

#include "GridLayoutWidget.h"

#include <tlUI/Label.h>
#include <tlUI/GridLayout.h>

namespace tl
{
    namespace examples
    {
        namespace ui_glfw
        {
            struct GridLayoutWidget::Private
            {
                std::shared_ptr<ui::GridLayout> layout;
            };

            void GridLayoutWidget::_init(
                const std::shared_ptr<system::Context>& context)
            {
                IWidget::_init("GridLayoutWidget", context);
                TLRENDER_P();

                auto label0 = ui::Label::create(context);
                label0->setText("Label 0");
                label0->setBackgroundRole(ui::ColorRole::Red);

                auto label1 = ui::Label::create(context);
                label1->setText("Label 1");
                label1->setFontRole(ui::FontRole::Title);
                label1->setBackgroundRole(ui::ColorRole::Green);

                auto label2 = ui::Label::create(context);
                label2->setText("Label 2");
                label2->setFontRole(ui::FontRole::Title);
                label2->setBackgroundRole(ui::ColorRole::Blue);
                label2->setHStretch(ui::Stretch::Expanding);

                auto label3 = ui::Label::create(context);
                label3->setText("Label 3");
                label3->setBackgroundRole(ui::ColorRole::Cyan);
                label3->setHStretch(ui::Stretch::Expanding);

                auto label4 = ui::Label::create(context);
                label4->setText("Label 4");
                label4->setBackgroundRole(ui::ColorRole::Magenta);
                label4->setHStretch(ui::Stretch::Expanding);
                label4->setVStretch(ui::Stretch::Expanding);

                auto label5 = ui::Label::create(context);
                label5->setText("Label 5");
                label5->setFontRole(ui::FontRole::Title);
                label5->setBackgroundRole(ui::ColorRole::Yellow);
                label5->setHStretch(ui::Stretch::Expanding);

                p.layout = ui::GridLayout::create(context, shared_from_this());
                label0->setParent(p.layout);
                label1->setParent(p.layout);
                label2->setParent(p.layout);
                label3->setParent(p.layout);
                label4->setParent(p.layout);
                label5->setParent(p.layout);
                p.layout->setGridPos(label0, 0, 0);
                p.layout->setGridPos(label1, 0, 1);
                p.layout->setGridPos(label2, 0, 2);
                p.layout->setGridPos(label3, 1, 0);
                p.layout->setGridPos(label4, 1, 1);
                p.layout->setGridPos(label5, 3, 3);
            }

            GridLayoutWidget::GridLayoutWidget() :
                _p(new Private)
            {}

            GridLayoutWidget::~GridLayoutWidget()
            {}

            std::shared_ptr<GridLayoutWidget> GridLayoutWidget::create(
                const std::shared_ptr<system::Context>& context)
            {
                auto out = std::shared_ptr<GridLayoutWidget>(new GridLayoutWidget);
                out->_init(context);
                return out;
            }

            void GridLayoutWidget::setGeometry(const math::BBox2i& value)
            {
                IWidget::setGeometry(value);
                _p->layout->setGeometry(value);
            }
        }
    }
}
