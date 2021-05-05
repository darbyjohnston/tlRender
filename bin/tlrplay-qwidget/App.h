// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021 Darby Johnston
// All rights reserved.

#pragma once

#include "MainWindow.h"

#include <tlrApp/IApp.h>

#include <tlrQt/TimeObject.h>
#include <tlrQt/TimelineObject.h>

#include <QApplication>
#include <QPointer>

namespace tlr
{
    //! Application.
    class App : public QApplication, public app::IApp
    {
        Q_OBJECT

    public:
        App(int& argc, char** argv);

    private Q_SLOTS:
        void _fileOpenCallback();
        void _fileOpenCallback(const QString&);
        void _fileCloseCallback();

    private:
        void _fileOpen(const std::string&);

        QPointer<qt::TimeObject> _timeObject;

        std::string _input;
        QPointer<qt::TimelineObject> _timeline;

        QPointer<MainWindow> _mainWindow;
    };
}
