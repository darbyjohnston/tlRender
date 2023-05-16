// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2023 Darby Johnston
// All rights reserved.

#pragma once

#include <tlApp/IApp.h>

#include <tlTimeline/IRender.h>

#include <QApplication>

namespace tl
{
    namespace timeline
    {
        class TimeUnitsModel;
    }

    namespace qt
    {
        class OutputDevice;
        class TimelineThumbnailObject;
        class TimeObject;
    }

    //! Playback application.
    namespace play
    {
        struct FilesModelItem;

        class ColorModel;
        class DevicesModel;
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

            //! Get the time units model.
            const std::shared_ptr<timeline::TimeUnitsModel>& timeUnitsModel() const;

            //! Get the time object.
            qt::TimeObject* timeObject() const;

            //! Get the settings object.
            SettingsObject* settingsObject() const;

            //! Get the thumbnail object.
            qt::TimelineThumbnailObject* thumbnailObject() const;

            //! Get the files model.
            const std::shared_ptr<FilesModel>& filesModel() const;

            //! Get the color model.
            const std::shared_ptr<ColorModel>& colorModel() const;

            //! Get the LUT options.
            const timeline::LUTOptions& lutOptions() const;

            //! Get the image options.
            const timeline::ImageOptions& imageOptions() const;

            //! Get the display options.
            const timeline::DisplayOptions& displayOptions() const;

            //! Get the audio volume.
            float volume() const;

            //! Get the audio mute.
            bool isMuted() const;

            //! Get the output device.
            qt::OutputDevice* outputDevice() const;

            //! Get the devices model.
            const std::shared_ptr<DevicesModel>& devicesModel() const;

        public Q_SLOTS:
            //! Open a file.
            void open(const QString&, const QString& = QString());

            //! Open a file dialog.
            void openDialog();

            //! Open a file with separate audio dialog.
            void openSeparateAudioDialog();

            //! Set the LUT options.
            void setLUTOptions(const tl::timeline::LUTOptions&);

            //! Set the image options.
            void setImageOptions(const tl::timeline::ImageOptions&);

            //! Set the display options.
            void setDisplayOptions(const tl::timeline::DisplayOptions&);

            //! Set the audio volume.
            void setVolume(float);

            //! Set the audio mute.
            void setMute(bool);

        Q_SIGNALS:
            //! This signal is emitted when the LUT options are changed.
            void lutOptionsChanged(const tl::timeline::LUTOptions&);

            //! This signal is emitted when the image options are changed.
            void imageOptionsChanged(const tl::timeline::ImageOptions&);

            //! This signal is emitted when the display options are changed.
            void displayOptionsChanged(const tl::timeline::DisplayOptions&);

            //! This signal is emitted when the audio volume is changed.
            void volumeChanged(float);

            //! This signal is emitted when the audio mute is changed.
            void muteChanged(bool);

        private Q_SLOTS:
            void _activeCallback(const std::vector<std::shared_ptr<tl::play::FilesModelItem> >&);
            void _settingsCallback();

        private:
            otime::RationalTime _cacheReadAhead() const;
            otime::RationalTime _cacheReadBehind() const;

            void _cacheUpdate();
            void _audioUpdate();

            TLRENDER_PRIVATE();
        };
    }
}
