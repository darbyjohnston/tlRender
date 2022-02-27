// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2022 Darby Johnston
// All rights reserved.

#pragma once

#include <tlQtWidget/FloatSlider.h>

namespace tl
{
    namespace examples
    {
        namespace widgets_qtwidget
        {
            class FloatSlider : public QWidget
            {
                Q_OBJECT

            public:
                FloatSlider(QWidget* parent = nullptr);

            private:
                std::vector<qt::widget::FloatSlider*> _sliders;
            };
        }
    }
}
