// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021 Darby Johnston
// All rights reserved.

#pragma once

#include "MainWindow.h"
#include "SettingsObject.h"

#include <tlrApp/IApp.h>

#include <tlrQt/TimeObject.h>
#include <tlrQt/TimelineObject.h>

#include <QApplication>

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
        void _fileClose();

    private:
        void _fileOpen(const QString&);

        qt::TimeObject* _timeObject = nullptr;
        SettingsObject* _settingsObject = nullptr;

        std::string _input;
        qt::TimelineObject* _timeline = nullptr;

        MainWindow* _mainWindow = nullptr;
    };
}
