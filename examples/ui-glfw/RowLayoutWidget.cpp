// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2023 Darby Johnston
// All rights reserved.

#include "RowLayoutWidget.h"

#include <tlUI/RowLayout.h>
#include <tlUI/Spacer.h>
#include <tlUI/TextLabel.h>

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

                auto textLabel0 = ui::TextLabel::create(context);
                textLabel0->setText("Text Label 0");

                auto textLabel1 = ui::TextLabel::create(context);
                textLabel1->setText("Text Label 0");
                imaging::FontInfo fontInfo;
                fontInfo.size = 32;
                textLabel1->setFontInfo(fontInfo);

                p.layout = ui::VerticalLayout::create(context);
                auto hLayout = ui::HorizontalLayout::create(context);
                hLayout->setParent(p.layout);
                textLabel0->setParent(hLayout);
                textLabel1->setParent(hLayout);
                p.layout->setParent(shared_from_this());
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
