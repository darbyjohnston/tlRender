// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2022 Darby Johnston
// All rights reserved.

#pragma once

#include "FilesModel.h"
#include "MainWindow.h"
#include "SettingsObject.h"

#include <tlrApp/IApp.h>

#include <tlrQt/TimeObject.h>
#include <tlrQt/TimelinePlayer.h>

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

        //! Get the time object.
        qt::TimeObject* timeObject() const;

        //! Get the settings object.
        SettingsObject* settingsObject() const;

        //! Get the file model.
        FilesModel* filesModel() const;

    public Q_SLOTS:
        //! Open a file.
        void open(const QString&, const QString& = QString());

        //! Open a file dialog.
        void open();

        //! Open a file with audio dialog.
        void openWithAudio();

    private Q_SLOTS:
        void _activeCallback(const std::vector<std::shared_ptr<FilesModelItem> >&);
        void _layerCallback(const std::shared_ptr<FilesModelItem>&, int);
        void _settingsCallback();

    private:
        void _settingsUpdate();

        std::string _input;
        Options _options;

        qt::TimeObject* _timeObject = nullptr;
        SettingsObject* _settingsObject = nullptr;
        FilesModel* _filesModel = nullptr;
        std::vector<std::shared_ptr<FilesModelItem> > _activeItems;
        std::vector<qt::TimelinePlayer*> _timelinePlayers;

        MainWindow* _mainWindow = nullptr;
    };
}
