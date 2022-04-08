// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2022 Darby Johnston
// All rights reserved.

#pragma once

#include <tlApp/IApp.h>

#include <tlTimeline/IRender.h>

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
        class FilesAModel;
        class FilesBModel;
        class SettingsObject;

        //! Application.
        class App : public QApplication, public app::IApp
        {
            Q_OBJECT

        public:
            App(
                int& argc,
                char** argv,
                const std::shared_ptr<system::Context>&);

            ~App() override;

            //! Get the time object.
            qt::TimeObject* timeObject() const;

            //! Get the settings object.
            SettingsObject* settingsObject() const;

            //! Get the files model.
            const std::shared_ptr<FilesModel>& filesModel() const;

            //! Get the color model.
            const std::shared_ptr<ColorModel>& colorModel() const;

            //! Get the image options.
            const timeline::ImageOptions& imageOptions() const;

            //! Get the display options.
            const timeline::DisplayOptions& displayOptions() const;

        public Q_SLOTS:
            //! Open a file.
            void open(const QString&, const QString& = QString());

            //! Open a file dialog.
            void openDialog();

            //! Open a file with audio dialog.
            void openWithAudioDialog();

            //! Set the image options.
            void setImageOptions(const tl::timeline::ImageOptions&);

            //! Set the display options.
            void setDisplayOptions(const tl::timeline::DisplayOptions&);

        Q_SIGNALS:
            //! This signal is emitted when the image options are changed.
            void imageOptionsChanged(const tl::timeline::ImageOptions&);

            //! This signal is emitted when the display options are changed.
            void displayOptionsChanged(const tl::timeline::DisplayOptions&);

        private Q_SLOTS:
            void _activeCallback(const std::vector<std::shared_ptr<tl::play::FilesModelItem> >&);
            void _settingsCallback();

        private:
            otime::RationalTime _cacheReadAhead() const;
            otime::RationalTime _cacheReadBehind() const;

            void _cacheUpdate();

            TLRENDER_PRIVATE();
        };
    }
}
