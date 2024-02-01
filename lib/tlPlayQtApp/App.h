// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2024 Darby Johnston
// All rights reserved.

#pragma once

#include <tlBaseApp/BaseApp.h>

#include <tlTimeline/TimeUnits.h>
#include <tlTimeline/IRender.h>

#include <tlIO/IO.h>

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
        class Settings;
        class ViewportModel;
    }

    namespace qt
    {
        class TimeObject;
        class TimelinePlayer;
    }

    namespace ui
    {
        class RecentFilesModel;
    }

    //! tlplay-qt application
    namespace play_qt
    {
        class FilesAModel;
        class FilesBModel;
        class MainWindow;

        //! Application.
        class App : public QApplication, public app::BaseApp
        {
            Q_OBJECT

        public:
            App(
                int& argc,
                char** argv,
                const std::shared_ptr<system::Context>&);

            virtual ~App();

            //! Get the time units model.
            const std::shared_ptr<timeline::TimeUnitsModel>& timeUnitsModel() const;

            //! Get the time object.
            qt::TimeObject* timeObject() const;

            //! Get the settings.
            const std::shared_ptr<play::Settings>& settings() const;

            //! Get the files model.
            const std::shared_ptr<play::FilesModel>& filesModel() const;

            //! Get the timeline players.
            const QVector<QSharedPointer<qt::TimelinePlayer> >& players() const;

            //! Get the active timeline players.
            QVector<QSharedPointer<qt::TimelinePlayer> > activePlayers() const;

            //! Get the recent files model.
            const std::shared_ptr<ui::RecentFilesModel>& recentFilesModel() const;

            //! Get the viewport model.
            const std::shared_ptr<play::ViewportModel>& viewportModel() const;

            //! Get the color model.
            const std::shared_ptr<play::ColorModel>& colorModel() const;

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
            //! This signal is emitted when the active players are changed.
            void activePlayersChanged(const QVector<QSharedPointer<qt::TimelinePlayer> >&);

            //! This signal is emitted when the secondary window active state is changed.
            void secondaryWindowChanged(bool);

        private Q_SLOTS:
            void _filesCallback(const std::vector<std::shared_ptr<tl::play::FilesModelItem> >&);
            void _activeCallback(const std::vector<std::shared_ptr<tl::play::FilesModelItem> >&);
            void _mainWindowDestroyedCallback();
            void _secondaryWindowDestroyedCallback();

        private:
            void _timerCallback();

            void _fileLogInit(const std::string&);
            void _settingsInit(const std::string&);
            void _modelsInit();
            void _devicesInit();
            void _observersInit();
            void _inputFilesInit();
            void _windowsInit();

            io::Options _ioOptions() const;
            otime::RationalTime _cacheReadAhead() const;
            otime::RationalTime _cacheReadBehind() const;

            void _settingsUpdate(const std::string&);
            void _cacheUpdate();
            void _viewUpdate(const math::Vector2i& pos, double zoom, bool frame);
            void _audioUpdate();

            TLRENDER_PRIVATE();
        };
    }
}
