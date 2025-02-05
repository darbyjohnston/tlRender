// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2024 Darby Johnston
// All rights reserved.

#include "NumericWidgets.h"

#include <tlUI/GridLayout.h>
#include <tlUI/GroupBox.h>
#include <tlUI/DoubleEditSlider.h>
#include <tlUI/IntEditSlider.h>
#include <tlUI/FloatEditSlider.h>
#include <tlUI/Label.h>
#include <tlUI/RowLayout.h>

#include <dtk/core/Format.h>

namespace tl
{
    namespace examples
    {
        namespace widgets
        {
            struct NumericWidgets::Private
            {
                std::shared_ptr<ui::RowLayout> layout;
            };

            void NumericWidgets::_init(
                const std::shared_ptr<dtk::Context>& context,
                const std::shared_ptr<IWidget>& parent)
            {
                IExampleWidget::_init(
                    "Numeric Widgets",
                    "tl::examples::widgets::NumericWidgets",
                    context,
                    parent);
                TLRENDER_P();

                const std::vector<dtk::RangeI> intRanges =
                {
                    dtk::RangeI(0, 10),
                    dtk::RangeI(0, 100),
                    dtk::RangeI(0, 1000),
                    dtk::RangeI(100, 200),
                    dtk::RangeI(-100, 200),
                    dtk::RangeI(-100, -200)
                };
                std::vector<std::shared_ptr<ui::Label> > intLabels;
                std::vector<std::shared_ptr<ui::IntEditSlider> > intEdits;
                for (const auto& i : intRanges)
                {
                    auto label = ui::Label::create(context);
                    label->setText(dtk::Format("{0} - {1}:").arg(i.min()).arg(i.max()));
                    intLabels.push_back(label);
                    auto edit = ui::IntEditSlider::create(context);
                    edit->setRange(i);
                    intEdits.push_back(edit);
                }

                const std::vector<dtk::RangeF> floatRanges =
                {
                    dtk::RangeF(0.F, 1.F),
                    dtk::RangeF(0.F, 10.F),
                    dtk::RangeF(0.F, 100.F),
                    dtk::RangeF(0.F, 1000.F),
                    dtk::RangeF(100.F, 200.F),
                    dtk::RangeF(-100.F, 200.F),
                    dtk::RangeF(-100.F, -200.F)
                };
                std::vector<std::shared_ptr<ui::Label> > floatLabels;
                std::vector<std::shared_ptr<ui::FloatEditSlider> > floatEdits;
                for (const auto& i : floatRanges)
                {
                    auto label = ui::Label::create(context);
                    label->setText(dtk::Format("{0} - {1}:").arg(i.min()).arg(i.max()));
                    floatLabels.push_back(label);
                    auto edit = ui::FloatEditSlider::create(context);
                    edit->setRange(i);
                    floatEdits.push_back(edit);
                }

                const std::vector<dtk::RangeD> doubleRanges =
                {
                    dtk::RangeD(0.0, 1.0),
                    dtk::RangeD(0.0, 10.0),
                    dtk::RangeD(0.0, 100.0),
                    dtk::RangeD(0.0, 1000.0),
                    dtk::RangeD(100.0, 200.0),
                    dtk::RangeD(-100.0, 200.0),
                    dtk::RangeD(-100.0, -200.0)
                };
                std::vector<std::shared_ptr<ui::Label> > doubleLabels;
                std::vector<std::shared_ptr<ui::DoubleEditSlider> > doubleEdits;
                for (const auto& i : doubleRanges)
                {
                    auto label = ui::Label::create(context);
                    label->setText(dtk::Format("{0} - {1}:").arg(i.min()).arg(i.max()));
                    doubleLabels.push_back(label);
                    auto edit = ui::DoubleEditSlider::create(context);
                    edit->setRange(i);
                    doubleEdits.push_back(edit);
                }
                
                p.layout = ui::VerticalLayout::create(context, shared_from_this());
                p.layout->setMarginRole(ui::SizeRole::Margin);
                auto groupBox = ui::GroupBox::create(context, p.layout);
                groupBox->setText("Integer Values");
                auto gridLayout = ui::GridLayout::create(context, groupBox);
                gridLayout->setSpacingRole(ui::SizeRole::SpacingSmall);
                for (int i = 0; i < intRanges.size(); ++i)
                {
                    intLabels[i]->setParent(gridLayout);
                    gridLayout->setGridPos(intLabels[i], i, 0);
                    intEdits[i]->setParent(gridLayout);
                    gridLayout->setGridPos(intEdits[i], i, 1);
                }
                groupBox = ui::GroupBox::create(context, p.layout);
                groupBox->setText("Floating Point Values");
                gridLayout = ui::GridLayout::create(context, groupBox);
                gridLayout->setSpacingRole(ui::SizeRole::SpacingSmall);
                for (int i = 0; i < floatRanges.size(); ++i)
                {
                    floatLabels[i]->setParent(gridLayout);
                    gridLayout->setGridPos(floatLabels[i], i, 0);
                    floatEdits[i]->setParent(gridLayout);
                    gridLayout->setGridPos(floatEdits[i], i, 1);
                }
                groupBox = ui::GroupBox::create(context, p.layout);
                groupBox->setText("Double Precision Floating Point Values");
                gridLayout = ui::GridLayout::create(context, groupBox);
                gridLayout->setSpacingRole(ui::SizeRole::SpacingSmall);
                for (int i = 0; i < doubleRanges.size(); ++i)
                {
                    doubleLabels[i]->setParent(gridLayout);
                    gridLayout->setGridPos(doubleLabels[i], i, 0);
                    doubleEdits[i]->setParent(gridLayout);
                    gridLayout->setGridPos(doubleEdits[i], i, 1);
                }
            }

            NumericWidgets::NumericWidgets() :
                _p(new Private)
            {}

            NumericWidgets::~NumericWidgets()
            {}

            std::shared_ptr<NumericWidgets> NumericWidgets::create(
                const std::shared_ptr<dtk::Context>& context,
                const std::shared_ptr<IWidget>& parent)
            {
                auto out = std::shared_ptr<NumericWidgets>(new NumericWidgets);
                out->_init(context, parent);
                return out;
            }

            void NumericWidgets::setGeometry(const dtk::Box2I& value)
            {
                IExampleWidget::setGeometry(value);
                _p->layout->setGeometry(value);
            }

            void NumericWidgets::sizeHintEvent(const ui::SizeHintEvent& event)
            {
                IExampleWidget::sizeHintEvent(event);
                _sizeHint = _p->layout->getSizeHint();
            }
        }
    }
}
