// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2025 Darby Johnston
// All rights reserved.

#pragma once

#include <tlPlay/SettingsModel.h>

#include <tlTimeline/TimeUnits.h>
#include <tlTimeline/IRender.h>

#include <tlIO/IO.h>

#include <dtk/core/IApp.h>

#include <QApplication>
#include <QSharedPointer>
#include <QVector>

namespace tl
{
#if defined(TLRENDER_BMD)
    namespace bmd
    {
        class DevicesModel;
        class OutputDevice;
    }
#endif // TLRENDER_BMD

    namespace play
    {
        struct FilesModelItem;

        class AudioModel;
        class ColorModel;
        class FilesModel;
        class RecentFilesModel;
        class RenderModel;
        class SettingsModel;
        class ViewportModel;
    }

    namespace qt
    {
        class TimeObject;
        class TimelinePlayer;
    }

    //! tlplay-qt application
    namespace play_qt
    {
        class FilesAModel;
        class FilesBModel;
        class MainWindow;

        //! Application.
        class App : public QApplication, public dtk::IApp
        {
            Q_OBJECT

        public:
            App(
                const std::shared_ptr<dtk::Context>&,
                int& argc,
                char** argv);

            virtual ~App();

            //! Get the time units model.
            const std::shared_ptr<timeline::TimeUnitsModel>& timeUnitsModel() const;

            //! Get the time object.
            qt::TimeObject* timeObject() const;

            //! Get the settings model.
            const std::shared_ptr<play::SettingsModel>& settingsModel() const;

            //! Get the files model.
            const std::shared_ptr<play::FilesModel>& filesModel() const;

            //! Get the recent files model.
            const std::shared_ptr<play::RecentFilesModel>& recentFilesModel() const;

            //! Reload the active files.
            void reload();

            //! Get the timeline player.
            const QSharedPointer<qt::TimelinePlayer>& player() const;

            //! Get the color model.
            const std::shared_ptr<play::ColorModel>& colorModel() const;

            //! Get the viewport model.
            const std::shared_ptr<play::ViewportModel>& viewportModel() const;

            //! Get the render model.
            const std::shared_ptr<play::RenderModel>& renderModel() const;

            //! Get the audio model.
            const std::shared_ptr<play::AudioModel>& audioModel() const;

            //! Get the main window.
            MainWindow* mainWindow() const;

#if defined(TLRENDER_BMD)
            //! Get the BMD devices model.
            const std::shared_ptr<bmd::DevicesModel>& bmdDevicesModel() const;

            //! Get the BMD output device.
            const std::shared_ptr<bmd::OutputDevice>& bmdOutputDevice() const;
#endif // TLRENDER_BMD

        public Q_SLOTS:
            //! Open a file.
            void open(const QString&, const QString& = QString());

            //! Open a file dialog.
            void openDialog();

            //! Open a file with separate audio dialog.
            void openSeparateAudioDialog();

            //! Set whether the secondary window is active.
            void setSecondaryWindow(bool);

        Q_SIGNALS:
            //! This signal is emitted when the timeline player is changed.
            void playerChanged(const QSharedPointer<qt::TimelinePlayer>&);

            //! This signal is emitted when the secondary window active state is changed.
            void secondaryWindowChanged(bool);

        private:
            void _modelsInit();
            void _devicesInit();
            void _observersInit();
            void _inputFilesInit();
            void _windowsInit();

            io::Options _ioOptions() const;

            void _timerUpdate();
            void _filesUpdate(const std::vector<std::shared_ptr<tl::play::FilesModelItem> >&);
            void _activeUpdate(const std::vector<std::shared_ptr<tl::play::FilesModelItem> >&);
            void _layersUpdate(const std::vector<int>&);
            void _cacheUpdate(const play::CacheOptions&);
            void _viewUpdate(const dtk::V2I& pos, double zoom, bool frame);
            void _audioUpdate();

            DTK_PRIVATE();
        };
    }
}
