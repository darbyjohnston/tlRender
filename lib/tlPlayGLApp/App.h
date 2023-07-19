// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2023 Darby Johnston
// All rights reserved.

#pragma once

#include <tlGLApp/IApp.h>

#include <tlUI/RecentFilesModel.h>

#include <tlPlay/FilesModel.h>

#include <tlTimeline/IRender.h>
#include <tlTimeline/Player.h>

#if defined(TLRENDER_USD)
#include <tlIO/USD.h>
#endif // TLRENDER_USD

struct GLFWwindow;

namespace tl
{
    //! OpenGL playback application.
    namespace play_gl
    {
        class MainWindow;
        class Settings;
        class ToolsModel;

        //! Application options.
        struct Options
        {
            std::string compareFileName;
            timeline::CompareOptions compareOptions;
            imaging::Size windowSize = imaging::Size(1280, 720);
            bool fullscreen = false;
            bool hud = true;
            double speed = 0.0;
            timeline::Playback playback = timeline::Playback::Stop;
            timeline::Loop loop = timeline::Loop::Loop;
            otime::RationalTime seek = time::invalidTime;
            otime::TimeRange inOutRange = time::invalidTimeRange;
            timeline::ColorConfigOptions colorConfigOptions;
            timeline::LUTOptions lutOptions;
#if defined(TLRENDER_USD)
            size_t usdRenderWidth = usd::RenderOptions().renderWidth;
            float usdComplexity = usd::RenderOptions().complexity;
            usd::DrawMode usdDrawMode = usd::RenderOptions().drawMode;
            bool usdEnableLighting = usd::RenderOptions().enableLighting;
            size_t usdStageCache = usd::RenderOptions().stageCacheCount;
            size_t usdDiskCache = usd::RenderOptions().diskCacheByteCount / memory::gigabyte;
#endif // TLRENDER_USD
        };

        //! Application.
        class App : public gl::IApp
        {
            TLRENDER_NON_COPYABLE(App);

        protected:
            void _init(
                int argc,
                char* argv[],
                const std::shared_ptr<system::Context>&);

            App();

        public:
            ~App();

            //! Create a new application.
            static std::shared_ptr<App> create(
                int argc,
                char* argv[],
                const std::shared_ptr<system::Context>&);

            //! Open a file.
            void open(
                const std::string&,
                const std::string& audioFileName = std::string());

            //! Open a file dialog.
            void openDialog();

            //! Open a file and separate audio dialog.
            void openSeparateAudioDialog();

            //! Get the settings.
            const std::shared_ptr<Settings>& getSettings() const;

            //! Get the files model.
            const std::shared_ptr<play::FilesModel>& getFilesModel() const;

            //! Get the recent files model.
            const std::shared_ptr<ui::RecentFilesModel>& getRecentFilesModel() const;

            //! Observe the active timeline players.
            std::shared_ptr<observer::IList<std::shared_ptr<timeline::Player> > > observeActivePlayers() const;

            //! Get the tools model.
            const std::shared_ptr<ToolsModel>& getToolsModel() const;

            //! Get the main window.
            const std::shared_ptr<MainWindow>& getMainWindow() const;

        protected:
            void _drop(const std::vector<std::string>&) override;
            void _tick() override;

        private:
            void _filesCallback(const std::vector<std::shared_ptr<play::FilesModelItem> >&);
            void _activeCallback(const std::vector<std::shared_ptr<play::FilesModelItem> >&);

            std::vector<std::shared_ptr<timeline::Player> > _getActivePlayers() const;
            otime::RationalTime _getCacheReadAhead() const;
            otime::RationalTime _getCacheReadBehind() const;

            void _cacheUpdate();
            void _audioUpdate();

            TLRENDER_PRIVATE();
        };
    }
}
