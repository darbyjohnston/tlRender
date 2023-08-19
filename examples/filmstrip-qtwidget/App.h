// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2023 Darby Johnston
// All rights reserved.

#pragma once

#include "MainWindow.h"

#include <tlQt/ContextObject.h>

#include <tlApp/IApp.h>

#include <QApplication>

namespace tl
{
    namespace examples
    {
        //! Example using the filmstrip widget.
        namespace filmstrip_qtwidget
        {
            //! Application.
            class App : public QApplication, public app::IApp
            {
                Q_OBJECT

            public:
                App(
                    int& argc,
                    char** argv,
                    const std::shared_ptr<system::Context>&);

                virtual ~App();

            private:
                std::string _input;
                QScopedPointer<qt::ContextObject> _contextObject;
                QScopedPointer<MainWindow> _mainWindow;
            };
        }
    }
}
