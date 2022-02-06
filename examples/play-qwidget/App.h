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
        const std::shared_ptr<FilesModel>& filesModel() const;

    public Q_SLOTS:
        //! Open a file.
        void open(const QString&, const QString& = QString());

        //! Open a file dialog.
        void open();

        //! Open a file with audio dialog.
        void openWithAudio();

    private Q_SLOTS:
        void _activeCallback(const std::vector<std::shared_ptr<FilesModelItem> >&);
        void _settingsCallback();

    private:
        void _settingsUpdate();

        std::string _input;
        Options _options;

        qt::TimeObject* _timeObject = nullptr;
        SettingsObject* _settingsObject = nullptr;
        std::shared_ptr<FilesModel> _filesModel;
        std::shared_ptr<observer::ListObserver<std::shared_ptr<FilesModelItem> > > _activeObserver;
        std::vector<std::shared_ptr<FilesModelItem> > _active;
        std::shared_ptr<observer::ListObserver<int> > _layersObserver;

        std::vector<qt::TimelinePlayer*> _timelinePlayers;

        MainWindow* _mainWindow = nullptr;
    };
}
