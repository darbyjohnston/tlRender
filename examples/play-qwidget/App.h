// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2022 Darby Johnston
// All rights reserved.

#pragma once

#include "FilesModel.h"
#include "LayersModel.h"
#include "MainWindow.h"
#include "SettingsObject.h"

#include <tlrApp/IApp.h>

#include <tlrQt/TimeObject.h>
#include <tlrQt/TimelinePlayer.h>

#include <tlrCore/OCIO.h>

#include <QApplication>

namespace tlr
{
    //! Application options.
    struct Options
    {
        imaging::ColorConfig colorConfig;
    };

    //! Application.
    class App : public QApplication, public app::IApp
    {
        Q_OBJECT

    public:
        App(int& argc, char** argv);
        ~App() override;

    public Q_SLOTS:
        //! Open a file.
        void open(const QString&);

        //! Open a file dialog.
        void open();

        //! Open a file with audio.
        void openWithAudio(const QString&, const QString&);

        //! Open a file with audio dialog.
        void openWithAudio();

        //! Close the current timeline.
        void close();

        //! Close all timelines.
        void closeAll();

    private Q_SLOTS:
        void _filesModelCallback(const std::shared_ptr<FilesModelItem>&);
        void _layersModelCallback(int);
        void _settingsCallback();

    private:
        void _settingsUpdate();

        std::string _input;
        Options _options;

        qt::TimeObject* _timeObject = nullptr;
        SettingsObject* _settingsObject = nullptr;
        FilesModel* _filesModel = nullptr;
        LayersModel* _layersModel = nullptr;
        std::shared_ptr<FilesModelItem> _currentFile;
        qt::TimelinePlayer* _timelinePlayer = nullptr;

        MainWindow* _mainWindow = nullptr;
    };
}
