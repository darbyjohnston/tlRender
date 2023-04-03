// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2023 Darby Johnston
// All rights reserved.

#include "RowLayoutWidget.h"

#include <tlUI/Label.h>
#include <tlUI/RowLayout.h>

namespace tl
{
    namespace examples
    {
        namespace ui_glfw
        {
            struct RowLayoutWidget::Private
            {
                std::shared_ptr<ui::RowLayout> layout;
            };

            void RowLayoutWidget::_init(
                const std::shared_ptr<system::Context>& context)
            {
                IWidget::_init("RowLayoutWidget", context);
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

                auto label5 = ui::Label::create(context);
                label5->setText("Label 5");
                label5->setFontRole(ui::FontRole::Title);
                label5->setBackgroundRole(ui::ColorRole::Yellow);
                label5->setHStretch(ui::Stretch::Expanding);

                p.layout = ui::VerticalLayout::create(context, shared_from_this());
                auto hLayout = ui::HorizontalLayout::create(context, p.layout);
                label0->setParent(hLayout);
                label1->setParent(hLayout);
                hLayout = ui::HorizontalLayout::create(context, p.layout);
                label2->setParent(hLayout);
                label3->setParent(hLayout);
                hLayout = ui::HorizontalLayout::create(context, p.layout);
                hLayout->setVStretch(ui::Stretch::Expanding);
                label4->setParent(hLayout);
                label5->setParent(hLayout);
            }

            RowLayoutWidget::RowLayoutWidget() :
                _p(new Private)
            {}

            RowLayoutWidget::~RowLayoutWidget()
            {}

            std::shared_ptr<RowLayoutWidget> RowLayoutWidget::create(
                const std::shared_ptr<system::Context>& context)
            {
                auto out = std::shared_ptr<RowLayoutWidget>(new RowLayoutWidget);
                out->_init(context);
                return out;
            }

            void RowLayoutWidget::setGeometry(const math::BBox2i& value)
            {
                IWidget::setGeometry(value);
                _p->layout->setGeometry(value);
            }
        }
    }
}
