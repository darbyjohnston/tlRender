// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2023 Darby Johnston
// All rights reserved.

#include "MainWindow.h"

#include "FloatSlider.h"
#include "IntSlider.h"

#include <QTabWidget>

namespace tl
{
    namespace examples
    {
        namespace widgets_qtwidget
        {
            MainWindow::MainWindow(const std::shared_ptr<system::Context>& context)
            {
                auto tabWidget = new QTabWidget;
                tabWidget->addTab(new FloatSlider, "FloatSlider");
                tabWidget->addTab(new IntSlider, "IntSlider");
                setCentralWidget(tabWidget);
            }
        }
    }
}
