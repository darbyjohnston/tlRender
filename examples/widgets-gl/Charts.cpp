// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2023 Darby Johnston
// All rights reserved.

#include "Charts.h"

#include <tlUI/GroupBox.h>
#include <tlUI/PieChart.h>
#include <tlUI/RowLayout.h>

namespace tl
{
    namespace examples
    {
        namespace widgets_gl
        {
            struct Charts::Private
            {
                std::shared_ptr<ui::RowLayout> layout;
            };

            void Charts::_init(
                const std::shared_ptr<system::Context>& context,
                const std::shared_ptr<IWidget>& parent)
            {
                IExampleWidget::_init(
                    "Charts",
                    "tl::examples::widgets_gl::Charts",
                    context,
                    parent);
                TLRENDER_P();

                auto pieChart0 = ui::PieChart::create(context);
                pieChart0->setData(
                    {
                        ui::PieChartData("0-20", 35, imaging::Color4f(.01F, .1F, .37F)),
                        ui::PieChartData("20-40", 25, imaging::Color4f(1.F, .73F, .27F)),
                        ui::PieChartData("40-60", 15, imaging::Color4f(78.F, .5F, 0.F)),
                        ui::PieChartData("60-80", 25, imaging::Color4f(1.F, .39F, .15F))
                    });
                auto pieChart1 = ui::PieChart::create(context);
                pieChart1->setData(
                    {
                        ui::PieChartData("Front", 20, imaging::Color4f(.92F, .5F, 0.F)),
                        ui::PieChartData("Side", 15, imaging::Color4f(.93F, .62F, .01F)),
                        ui::PieChartData("Back", 10, imaging::Color4f(.13F, .3F, .44F)),
                        ui::PieChartData("Above", 25, imaging::Color4f(.07F, .48F, .53F)),
                        ui::PieChartData("Below", 30, imaging::Color4f(.67F, .92F, .93F))
                    });
                pieChart1->setSizeMult(10);

                p.layout = ui::VerticalLayout::create(context, shared_from_this());
                p.layout->setMarginRole(ui::SizeRole::Margin);
                auto groupBox = ui::GroupBox::create(context, p.layout);
                groupBox->setText("Pie Charts");
                auto hLayout = ui::HorizontalLayout::create(context, groupBox);
                pieChart0->setParent(hLayout);
                pieChart1->setParent(hLayout);
            }

            Charts::Charts() :
                _p(new Private)
            {}

            Charts::~Charts()
            {}

            std::shared_ptr<Charts> Charts::create(
                const std::shared_ptr<system::Context>& context,
                const std::shared_ptr<IWidget>& parent)
            {
                auto out = std::shared_ptr<Charts>(new Charts);
                out->_init(context, parent);
                return out;
            }

            void Charts::setGeometry(const math::BBox2i& value)
            {
                IExampleWidget::setGeometry(value);
                _p->layout->setGeometry(value);
            }

            void Charts::sizeHintEvent(const ui::SizeHintEvent& event)
            {
                IExampleWidget::sizeHintEvent(event);
                _sizeHint = _p->layout->getSizeHint();
            }
        }
    }
}
