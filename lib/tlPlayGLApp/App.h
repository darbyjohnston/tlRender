// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2023 Darby Johnston
// All rights reserved.

#include <tlGLApp/IApp.h>

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

            //! Get the files model.
            const std::shared_ptr<play::FilesModel>& getFilesModel() const;

            //! Observe the current timeline player.
            std::shared_ptr<observer::IValue<std::shared_ptr<timeline::Player> > > observePlayer() const;

            //! Observe the list of timeline players.
            std::shared_ptr<observer::IList<std::shared_ptr<timeline::Player> > > observePlayers() const;

            //! Get the main window.
            const std::shared_ptr<MainWindow>& getMainWindow() const;

        protected:
            void _drop(const std::vector<std::string>&) override;
            void _tick() override;

        private:
            void _activeCallback(const std::vector<std::shared_ptr<play::FilesModelItem> >&);

            otime::RationalTime _cacheReadAhead() const;
            otime::RationalTime _cacheReadBehind() const;

            void _cacheUpdate();
            void _audioUpdate();

            TLRENDER_PRIVATE();
        };
    }
}
