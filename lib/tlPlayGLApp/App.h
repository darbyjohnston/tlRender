// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2023 Darby Johnston
// All rights reserved.

#pragma once

#include <tlGLApp/IApp.h>

#include <tlTimeline/Player.h>

namespace tl
{
    namespace ui
    {
        class RecentFilesModel;
    }

    namespace play
    {
        struct FilesModelItem;

        class AudioModel;
        class ColorModel;
        class FilesModel;
        class Settings;
        class ViewportModel;
    }

    //! "tlplay-gl" application.
    namespace play_gl
    {
        class MainWindow;
        class ToolsModel;

        //! Application.
        class App : public gl::IApp
        {
            TLRENDER_NON_COPYABLE(App);

        protected:
            void _init(
                const std::vector<std::string>&,
                const std::shared_ptr<system::Context>&);

            App();

        public:
            ~App();

            //! Create a new application.
            static std::shared_ptr<App> create(
                const std::vector<std::string>&,
                const std::shared_ptr<system::Context>&);

            //! Open a file.
            void open(
                const file::Path& path,
                const file::Path& audioPath = file::Path());

            //! Open a file dialog.
            void openDialog();

            //! Open a file and separate audio dialog.
            void openSeparateAudioDialog();

            //! Get the settings.
            const std::shared_ptr<play::Settings>& getSettings() const;

            //! Get the files model.
            const std::shared_ptr<play::FilesModel>& getFilesModel() const;

            //! Observe the active timeline players.
            std::shared_ptr<observer::IList<std::shared_ptr<timeline::Player> > > observeActivePlayers() const;

            //! Get the viewport model.
            const std::shared_ptr<play::ViewportModel>& getViewportModel() const;

            //! Get the color model.
            const std::shared_ptr<play::ColorModel>& getColorModel() const;

            //! Get the audio model.
            const std::shared_ptr<play::AudioModel>& getAudioModel() const;

            //! Get the tools model.
            const std::shared_ptr<ToolsModel>& getToolsModel() const;

            //! Observe whether the main window is in full screen mode.
            std::shared_ptr<observer::IValue<bool> > observeFullScreen() const;

            //! Observe whether the main window is floating on top.
            std::shared_ptr<observer::IValue<bool> > observeFloatOnTop() const;

            //! Get the main window.
            const std::shared_ptr<MainWindow>& getMainWindow() const;

        protected:
            void _drop(const std::vector<std::string>&) override;
            void _tick() override;

        private:
            void _fileLogInit(const std::string&);
            void _settingsInit(const std::string&);
            void _modelsInit();
            void _observersInit();
            void _inputFilesInit();
            void _mainWindowInit();

            io::Options _getIOOptions() const;
            std::vector<std::shared_ptr<timeline::Player> > _getActivePlayers() const;
            otime::RationalTime _getCacheReadAhead() const;
            otime::RationalTime _getCacheReadBehind() const;

            void _filesCallback(const std::vector<std::shared_ptr<play::FilesModelItem> >&);
            void _activeCallback(const std::vector<std::shared_ptr<play::FilesModelItem> >&);

            void _settingsUpdate(const std::string&);
            void _cacheUpdate();
            void _audioUpdate();

            TLRENDER_PRIVATE();
        };
    }
}
