// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2024 Darby Johnston
// All rights reserved.

#pragma once

#include "MainWindow.h"

#include <tlQt/ContextObject.h>

#include <QApplication>

namespace tl
{
    namespace examples
    {
        //! Example Qt widgets application.
        namespace widgets_qtwidget
        {
            class App : public QApplication
            {
                Q_OBJECT

            public:
                App(
                    const std::shared_ptr<dtk::Context>&,
                    int& argc,
                    char** argv);

            private:
                QScopedPointer<qt::ContextObject> _contextObject;
                QScopedPointer<MainWindow> _mainWindow;
            };
        }
    }
}
