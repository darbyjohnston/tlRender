// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2022 Darby Johnston
// All rights reserved.

#include "App.h"

#include <tlQWidget/Style.h>

namespace tl
{
    namespace widgets_qwidget
    {
        App::App(int& argc, char** argv) :
            QApplication(argc, argv)
        {
            _context = core::Context::create();

            setStyle("Fusion");
            setPalette(qwidget::darkStyle());
            setStyleSheet(qwidget::styleSheet());

            _mainWindow = new MainWindow(_context);
            _mainWindow->show();
        }
    }
}
