// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2022 Darby Johnston
// All rights reserved.

#include "Util.h"

#include <tlApp/IApp.h>

#include <tlTimeline/IRender.h>
#include <tlTimeline/TimelinePlayer.h>

#include <tlCore/OCIO.h>

struct GLFWwindow;

namespace tl
{
    namespace examples
    {
        //! Example GLFW playback application.
        namespace play_glfw
        {
            //! Application options.
            struct Options
            {
                imaging::Size windowSize = imaging::Size(1280, 720);
                bool fullScreen = false;
                bool hud = true;
                bool startPlayback = true;
                bool loopPlayback = true;
                imaging::ColorConfig colorConfig;
            };

            //! Application.
            class App : public app::IApp
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

                //! Run the application.
                void run();

                //! Exit the application.
                void exit();

            private:
                void _fullscreenWindow();
                void _normalWindow();
                void _fullscreenCallback(bool);
                static void _frameBufferSizeCallback(GLFWwindow*, int, int);
                static void _windowContentScaleCallback(GLFWwindow*, float, float);
                static void _keyCallback(GLFWwindow*, int, int, int, int);

                void _printShortcutsHelp();

                void _tick();
                void _hudUpdate();

                void _drawVideo();
                void _drawHUD();
                void _hudCallback(bool);

                void _playbackCallback(timeline::Playback);
                void _loopPlaybackCallback(timeline::Loop);

                std::string _input;
                Options _options;

                std::shared_ptr<timeline::TimelinePlayer> _timelinePlayer;

                GLFWwindow* _glfwWindow = nullptr;
                math::Vector2i _windowPos;
                imaging::Size _frameBufferSize;
                math::Vector2f _contentScale;
                std::shared_ptr<timeline::IRender> _render;
                bool _renderDirty = true;
                timeline::VideoData _videoData;
                std::map<HUDElement, std::string> _hudLabels;

                bool _running = true;
            };
        }
    }
}
