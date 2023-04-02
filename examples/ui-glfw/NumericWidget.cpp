// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2023 Darby Johnston
// All rights reserved.

#include "NumericWidget.h"

#include <tlUI/GridLayout.h>
#include <tlUI/GroupBox.h>
#include <tlUI/IntEdit.h>
#include <tlUI/IntSlider.h>
#include <tlUI/FloatEdit.h>
#include <tlUI/FloatSlider.h>
#include <tlUI/Label.h>
#include <tlUI/RowLayout.h>

#include <tlCore/StringFormat.h>

namespace tl
{
    namespace examples
    {
        namespace ui_glfw
        {
            struct NumericWidget::Private
            {
                std::shared_ptr<ui::RowLayout> layout;
            };

            void NumericWidget::_init(
                const std::shared_ptr<system::Context>& context)
            {
                IWidget::_init("NumericWidget", context);
                TLRENDER_P();

                const std::vector<math::IntRange> intRanges =
                {
                    math::IntRange(0, 10),
                    math::IntRange(0, 100),
                    math::IntRange(0, 1000),
                    math::IntRange(100, 200)
                };
                std::vector<std::shared_ptr<ui::Label> > intLabels;
                std::vector<std::shared_ptr<ui::IntEdit> > intEdits;
                std::vector<std::shared_ptr<ui::IntSlider> > intSliders;
                for (const auto& i : intRanges)
                {
                    auto label = ui::Label::create(context);
                    label->setText(string::Format("{0}-{1}:").arg(i.getMin()).arg(i.getMax()));
                    intLabels.push_back(label);
                    auto edit = ui::IntEdit::create(context);
                    auto model = edit->getModel();
                    model->setRange(i);
                    edit->setDigits(math::digits(i.getMax()));
                    intEdits.push_back(edit);
                    auto slider = ui::IntSlider::create(context);
                    slider->setModel(model);
                    slider->setStretch(ui::Stretch::Expanding, ui::Orientation::Horizontal);
                    intSliders.push_back(slider);
                }

                const std::vector<math::FloatRange> floatRanges =
                {
                    math::FloatRange(0.F, 1.F),
                    math::FloatRange(0.F, 10.F),
                    math::FloatRange(0.F, 100.F),
                    math::FloatRange(0.F, 1000.F),
                    math::FloatRange(-100.F, 200.F)
                };
                std::vector<std::shared_ptr<ui::Label> > floatLabels;
                std::vector<std::shared_ptr<ui::FloatEdit> > floatEdits;
                std::vector<std::shared_ptr<ui::FloatSlider> > floatSliders;
                for (const auto& i : floatRanges)
                {
                    auto label = ui::Label::create(context);
                    label->setText(string::Format("{0}-{1}:").arg(i.getMin()).arg(i.getMax()));
                    floatLabels.push_back(label);
                    auto edit = ui::FloatEdit::create(context);
                    auto model = edit->getModel();
                    model->setRange(i);
                    edit->setDigits(math::digits(i.getMax()));
                    floatEdits.push_back(edit);
                    auto slider = ui::FloatSlider::create(context);
                    slider->setModel(model);
                    slider->setStretch(ui::Stretch::Expanding, ui::Orientation::Horizontal);
                    floatSliders.push_back(slider);
                }

                p.layout = ui::VerticalLayout::create(context, shared_from_this());
                auto groupBox = ui::GroupBox::create(context, p.layout);
                groupBox->setText("Integer Values");
                auto gridLayout = ui::GridLayout::create(context, groupBox);
                for (int i = 0; i < intRanges.size(); ++i)
                {
                    intLabels[i]->setParent(gridLayout);
                    gridLayout->setGridPos(intLabels[i], i, 0);
                    intEdits[i]->setParent(gridLayout);
                    gridLayout->setGridPos(intEdits[i], i, 1);
                    intSliders[i]->setParent(gridLayout);
                    gridLayout->setGridPos(intSliders[i], i, 2);
                }
                groupBox = ui::GroupBox::create(context, p.layout);
                groupBox->setText("Floating Point Values");
                gridLayout = ui::GridLayout::create(context, groupBox);
                for (int i = 0; i < floatRanges.size(); ++i)
                {
                    floatLabels[i]->setParent(gridLayout);
                    gridLayout->setGridPos(floatLabels[i], i, 0);
                    floatEdits[i]->setParent(gridLayout);
                    gridLayout->setGridPos(floatEdits[i], i, 1);
                    floatSliders[i]->setParent(gridLayout);
                    gridLayout->setGridPos(floatSliders[i], i, 2);
                }
            }

            NumericWidget::NumericWidget() :
                _p(new Private)
            {}

            NumericWidget::~NumericWidget()
            {}

            std::shared_ptr<NumericWidget> NumericWidget::create(
                const std::shared_ptr<system::Context>& context)
            {
                auto out = std::shared_ptr<NumericWidget>(new NumericWidget);
                out->_init(context);
                return out;
            }

            void NumericWidget::setGeometry(const math::BBox2i& value)
            {
                IWidget::setGeometry(value);
                _p->layout->setGeometry(value);
            }
        }
    }
}
