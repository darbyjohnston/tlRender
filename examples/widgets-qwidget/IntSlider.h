// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2022 Darby Johnston
// All rights reserved.

#pragma once

#include <tlQWidget/IntSlider.h>

namespace tl
{
    namespace widgets_qwidget
    {
        class IntSlider : public QWidget
        {
            Q_OBJECT

        public:
            IntSlider(QWidget* parent = nullptr);

        private:
            std::vector<qwidget::IntSlider*> _sliders;
        };
    }
}
