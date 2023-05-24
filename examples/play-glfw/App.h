// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2023 Darby Johnston
// All rights reserved.

#include <tlGLFWApp/IApp.h>

#include <tlTimeline/IRender.h>
#include <tlTimeline/Player.h>

struct GLFWwindow;

namespace tl
{
    namespace examples
    {
        //! Example GLFW playback application.
        namespace play_glfw
        {
            class MainWindow;

            //! HUD elements.
            enum class HUDElement
            {
                UpperLeft,
                UpperRight,
                LowerLeft,
                LowerRight
            };

            //! Application options.
            struct Options
            {
                std::string compareFileName;
                timeline::CompareOptions compareOptions;
                imaging::Size windowSize = imaging::Size(1280, 720);
                bool fullscreen = false;
                bool hud = true;
                timeline::Playback playback = timeline::Playback::Forward;
                timeline::Loop loop = timeline::Loop::Loop;
                otime::RationalTime seek = time::invalidTime;
                otime::TimeRange inOutRange = time::invalidTimeRange;
                timeline::ColorConfigOptions colorConfigOptions;
                timeline::LUTOptions lutOptions;
            };

            //! Application.
            class App : public glfw::IApp
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

            protected:
                void _tick() override;

            private:
                std::string _input;
                Options _options;
                std::shared_ptr<timeline::Player> _player;
                std::shared_ptr<MainWindow> _mainWindow;
            };
        }
    }
}
