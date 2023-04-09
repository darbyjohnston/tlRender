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
        //! Example GLFW playback application.
        namespace play_glfw
        {
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
                void _setFullscreenWindow(bool);
                void _fullscreenCallback(bool);
                static void _frameBufferSizeCallback(GLFWwindow*, int, int);
                static void _windowContentScaleCallback(GLFWwindow*, float, float);
                static void _keyCallback(GLFWwindow*, int, int, int, int);

                void _printShortcutsHelp();

                void _tick();

                void _hudUpdate();
                void _hudCallback(bool);
                void _drawHUD();
                void _drawHUDLabel(
                    const std::string& text,
                    const imaging::FontInfo&,
                    HUDElement);

                void _playbackCallback(timeline::Playback);
                void _loopPlaybackCallback(timeline::Loop);

                std::string _input;
                Options _options;

                std::shared_ptr<timeline::TimelinePlayer> _timelinePlayer;

                GLFWwindow* _glfwWindow = nullptr;
                imaging::Size _windowSize;
                math::Vector2i _windowPos;
                bool _fullscreen = false;
                imaging::Size _frameBufferSize;
                math::Vector2f _contentScale = math::Vector2f(1.F, 1.F);
                bool _hud = false;
                std::shared_ptr<imaging::FontSystem> _fontSystem;
                std::shared_ptr<timeline::IRender> _render;
                bool _renderDirty = true;
                timeline::VideoData _videoData;
                std::map<HUDElement, std::string> _hudLabels;

                bool _running = true;
            };
        }
    }
}
