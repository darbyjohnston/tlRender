// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2023 Darby Johnston
// All rights reserved.

#include <tlApp/IApp.h>

#include <tlTimeline/IRender.h>
#include <tlTimeline/TimelinePlayer.h>

#include <tlCore/FontSystem.h>

struct GLFWwindow;

namespace tl
{
    namespace examples
    {
        //! Example GLFW rendering application.
        namespace render_glfw
        {
            //! Application options.
            struct Options
            {
                std::string compareFileName;
                imaging::Size windowSize = imaging::Size(1280, 720);
                bool fullScreen = false;
                bool hud = true;
                timeline::Playback playback = timeline::Playback::Forward;
                otime::RationalTime seek = time::invalidTime;
                otime::TimeRange inOutRange = time::invalidTimeRange;
                timeline::ColorConfigOptions colorConfigOptions;
                timeline::LUTOptions lutOptions;
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

                void _drawVideo();
                void _drawVideo(
                    const math::BBox2i& bbox,
                    const timeline::CompareOptions&,
                    float rotation);

                void _hudUpdate();
                void _hudCallback(bool);
                void _drawHUD();

                void _playbackCallback(timeline::Playback);

                std::string _input;
                Options _options;

                std::vector<std::shared_ptr<timeline::TimelinePlayer> > _timelinePlayers;
                std::vector<imaging::Size> _videoSizes;

                GLFWwindow* _glfwWindow = nullptr;
                math::Vector2i _windowPos;
                imaging::Size _frameBufferSize;
                math::Vector2f _contentScale = math::Vector2f(1.F, 1.F);
                timeline::CompareOptions _compareOptions;
                float _rotation = 0.F;
                std::shared_ptr<imaging::FontSystem> _fontSystem;
                std::shared_ptr<timeline::IRender> _render;
                bool _renderDirty = true;
                std::vector<timeline::VideoData> _videoData;
                std::chrono::steady_clock::time_point _startTime;

                bool _running = true;
            };
        }
    }
}
