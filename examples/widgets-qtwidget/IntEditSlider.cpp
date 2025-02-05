// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2024 Darby Johnston
// All rights reserved.

#include "IntEditSlider.h"

#include <QBoxLayout>

namespace tl
{
    namespace examples
    {
        namespace widgets_qtwidget
        {
            IntEditSlider::IntEditSlider(QWidget* parent) :
                QWidget(parent)
            {
                for (size_t i = 0; i < 4; ++i)
                {
                    _sliders.push_back(new qtwidget::IntEditSlider);
                }
                _sliders[1]->setRange(dtk::RangeI(0, 10));
                _sliders[1]->setValue(5);
                _sliders[2]->setRange(dtk::RangeI(-10, 10));
                _sliders[2]->setValue(-5);
                _sliders[3]->setRange(dtk::RangeI(-10000, 10000));
                _sliders[3]->setValue(5000);
                _sliders[3]->setDefaultValue(5000);
                _sliders[3]->setSingleStep(10);
                _sliders[3]->setPageStep(100);

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
