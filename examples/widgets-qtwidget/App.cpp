// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2023 Darby Johnston
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
                int& argc,
                char** argv,
                const std::shared_ptr<system::Context>& context) :
                QApplication(argc, argv)
            {
                setStyle("Fusion");
                setPalette(qtwidget::darkStyle());
                setStyleSheet(qtwidget::styleSheet());

                _contextObject = new qt::ContextObject(context, this);

                _mainWindow = new MainWindow(context);
                _mainWindow->show();
            }
        }
    }
}
