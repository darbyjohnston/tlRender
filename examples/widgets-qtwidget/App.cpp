// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2024 Darby Johnston
// All rights reserved.

#include "App.h"

#include <tlQtWidget/Style.h>

namespace tl
{
    namespace examples
    {
        namespace widgets_qtwidget
        {
            App::App(
                const std::shared_ptr<dtk::Context>& context,
                int& argc,
                char** argv) :
                QApplication(argc, argv)
            {
                setStyle("Fusion");
                setPalette(qtwidget::darkStyle());
                setStyleSheet(qtwidget::styleSheet());

                _contextObject.reset(new qt::ContextObject(context, this));

                _mainWindow.reset(new MainWindow(context));
                _mainWindow->show();
            }
        }
    }
}
