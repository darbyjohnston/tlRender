// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2022 Darby Johnston
// All rights reserved.

#pragma once

#include <tlApp/IApp.h>

#include <tlCore/OCIO.h>

#include <QApplication>

namespace tl
{
    namespace qt
    {
        class TimeObject;
    }

    //! Playback application.
    namespace play
    {
        struct FilesModelItem;

        class ColorModel;
        class FilesModel;
        class SettingsObject;

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

            //! Get the files model.
            const std::shared_ptr<FilesModel>& filesModel() const;

            //! Get the color model.
            const std::shared_ptr<ColorModel>& colorModel() const;

        public Q_SLOTS:
            //! Open a file.
            void open(const QString&, const QString& = QString());

            //! Open a file dialog.
            void openDialog();

            //! Open a file with audio dialog.
            void openWithAudioDialog();

        private Q_SLOTS:
            void _activeCallback(const std::vector<std::shared_ptr<tl::play::FilesModelItem> >&);
            void _settingsCallback();

        private:
            void _settingsUpdate();

            TLRENDER_PRIVATE();
        };
    }
}
