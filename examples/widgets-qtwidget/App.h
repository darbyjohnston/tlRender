// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2022 Darby Johnston
// All rights reserved.

#pragma once

#include "MainWindow.h"

#include <tlCore/Context.h>

#include <QApplication>

namespace tl
{
    namespace examples
    {
        //! Example showing various widgets.
        namespace widgets_qtwidget
        {
            class App : public QApplication
            {
                Q_OBJECT

            public:
                App(int& argc, char** argv);

            private:
                std::shared_ptr<core::system::Context> _context;
                MainWindow* _mainWindow = nullptr;
            };
        }
    }
}
