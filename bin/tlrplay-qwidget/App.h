// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021 Darby Johnston
// All rights reserved.

#pragma once

#include "MainWindow.h"
#include "SettingsObject.h"

#include <tlrApp/IApp.h>

#include <tlrQt/TimeObject.h>
#include <tlrQt/TimelinePlayer.h>

#include <QApplication>

namespace tlr
{
    //! Application.
    class App : public QApplication, public app::IApp
    {
        Q_OBJECT

    public:
        App(int& argc, char** argv);

    public Q_SLOTS:
        //! Open a timeline.
        void open(const QString&);

        //! Close a timeline.
        void close(tlr::qt::TimelinePlayer*);

        //! Close all of the timelines.
        void closeAll();

    Q_SIGNALS:
        //! This signal is emitted when a timeline is opened.
        void opened(tlr::qt::TimelinePlayer*);

        //! This signal is emitted when a timeline is closed.
        void closed(tlr::qt::TimelinePlayer*);

    private:
        qt::TimeObject* _timeObject = nullptr;
        SettingsObject* _settingsObject = nullptr;

        std::string _input;
        QList<qt::TimelinePlayer*> _timelinePlayers;

        MainWindow* _mainWindow = nullptr;
    };
}
