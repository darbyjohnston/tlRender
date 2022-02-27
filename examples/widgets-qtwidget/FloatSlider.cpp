// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2022 Darby Johnston
// All rights reserved.

#include "FloatSlider.h"

#include <QBoxLayout>

using namespace tl::core;

namespace tl
{
    namespace examples
    {
        namespace widgets_qtwidget
        {
            FloatSlider::FloatSlider(QWidget* parent) :
                QWidget(parent)
            {
                for (size_t i = 0; i < 4; ++i)
                {
                    _sliders.push_back(new qt::widget::FloatSlider);
                }
                _sliders[1]->setRange(math::FloatRange(0.F, 10.F));
                _sliders[1]->setValue(5.F);
                _sliders[2]->setRange(math::FloatRange(-10.F, 10.F));
                _sliders[2]->setValue(-5.F);
                _sliders[3]->setRange(math::FloatRange(-10000.F, 10000.F));
                _sliders[3]->setValue(5000.F);
                _sliders[3]->setDefaultValue(5000.F);
                _sliders[3]->setSingleStep(10.F);
                _sliders[3]->setPageStep(100.F);

                auto layout = new QVBoxLayout;
                for (size_t i = 0; i < _sliders.size(); ++i)
                {
                    layout->addWidget(_sliders[i]);
                }
                setLayout(layout);
            }
        }
    }
}
