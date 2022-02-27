// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2022 Darby Johnston
// All rights reserved.

#include "App.h"

#include <tlQtWidget/Style.h>

#include <tlIO/IOSystem.h>

namespace tl
{
    namespace examples
    {
        namespace widgets_qtwidget
        {
            App::App(int& argc, char** argv) :
                QApplication(argc, argv)
            {
                _context = system::Context::create();
                _context->addSystem(io::System::create(_context));

                setStyle("Fusion");
                setPalette(qtwidget::darkStyle());
                setStyleSheet(qtwidget::styleSheet());

                _mainWindow = new MainWindow(_context);
                _mainWindow->show();
            }
        }
    }
}
