// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021 Darby Johnston
// All rights reserved.

#include "Util.h"

#include <tlrApp/IApp.h>

#include <tlrGL/FontSystem.h>
#include <tlrGL/Render.h>

#include <tlrCore/Timeline.h>

struct GLFWwindow;

namespace tlr
{
    //! Application options.
    struct Options
    {
        bool fullScreen = false;
        bool hud = true;
        bool startPlayback = true;
        bool loopPlayback = true;
    };

    //! Application.
    class App : public app::IApp
    {
        TLR_NON_COPYABLE(App);

    protected:
        void _init(int argc, char* argv[]);
        App();

    public:
        ~App();

        //! Create a new application.
        static std::shared_ptr<App> create(int argc, char* argv[]);

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
        void _updateHUD();

        void _renderVideo();
        void _renderHUD();
        void _hudCallback(bool);

        void _playbackCallback(timeline::Playback);
        void _loopPlaybackCallback(timeline::Loop);
        void _seekCallback(const otime::RationalTime&);

        std::string _input;
        Options _options;

        std::shared_ptr<timeline::Timeline> _timeline;

        GLFWwindow* _glfwWindow = nullptr;
        math::Vector2i _windowPos;
        imaging::Size _windowSize = imaging::Size(640, 360);
        imaging::Size _frameBufferSize;
        math::Vector2f _contentScale;
        std::shared_ptr<gl::FontSystem> _fontSystem;
        std::shared_ptr<gl::Render> _render;
        bool _renderDirty = true;
        std::shared_ptr<imaging::Image> _currentImage;
        std::map<HUDElement, std::string> _hudLabels;

        bool _running = true;
    };
}
