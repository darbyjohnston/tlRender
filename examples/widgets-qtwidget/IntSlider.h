// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2022 Darby Johnston
// All rights reserved.

#pragma once

#include <tlQtWidget/IntSlider.h>

namespace tl
{
    namespace examples
    {
        namespace widgets_qtwidget
        {
            class IntSlider : public QWidget
            {
                Q_OBJECT

            public:
                IntSlider(QWidget* parent = nullptr);

            private:
                std::vector<qtwidget::IntSlider*> _sliders;
            };
        }
    }
}
