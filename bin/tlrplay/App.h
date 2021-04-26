// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021 Darby Johnston
// All rights reserved.

#include <tlrApp/IApp.h>
#include <tlrApp/Util.h>

#include <tlrRender/FontSystem.h>
#include <tlrRender/Render.h>

#include <tlrAV/IO.h>

#include <tlrTimeline/Util.h>

#include <opentimelineio/clip.h>
#include <opentimelineio/timeline.h>
#include <opentimelineio/track.h>

#include <memory>
#include <string>
#include <vector>

struct GLFWwindow;

namespace tlr
{
    //! Options
    struct Options
    {
        float windowScale = 1.F;
        bool fullScreen = false;
        bool hud = true;
        bool startPlayback = true;
        bool loopPlayback = true;
        size_t ioVideoQueueSize = 10;
        bool verbose = false;
        bool help = false;
    };

    //! Application
    class App : app::IApp
    {
        TLR_NON_COPYABLE(App);

    protected:
        App();

    public:
        ~App();

        //! Create a new application.
        static std::shared_ptr<App> create(int argc, char* argv[]);

        //! Run the application.
        int run();

        //! Exit the application.
        void exit();

    private:
        int _parseCmdLine();

        void _readTimeline();

        void _createWindow();
        void _destroyWindow();
        void _fullscreenWindow();
        void _normalWindow();
        void _fullscreenCallback(bool);
        static void _keyCallback(GLFWwindow*, int, int, int, int);
        void _shortcutsHelp();

        void _tick();

        void _updateReaders();

        void _renderVideo();
        void _renderHUD();
        void _hudCallback(bool);

        enum class Playback
        {
            Stop,
            Forward
        };
        void _forwardPlayback();
        void _stopPlayback();
        void _playbackCallback(Playback);
        void _loopPlaybackCallback(bool);
        void _seek(const otime::RationalTime&);

        void _print(const std::string&);
        void _printVerbose(const std::string&);
        void _printError(const std::string&);

        std::string _input;
        Options _options;

        std::shared_ptr<av::io::System> _ioSystem;
        otio::SerializableObject::Retainer<otio::Timeline> _timeline;
        otio::SerializableObject::Retainer<otio::Track> _flattenedTimeline;
        otime::RationalTime _duration;
        imaging::Info _info;

        GLFWwindow* _glfwWindow = nullptr;
        math::Vector2i _windowPos;
        imaging::Size _windowSize;
        imaging::Size _frameBufferSize;
        math::Vector2f _contentScale;

        typedef std::pair<otio::SerializableObject::Retainer<otio::Clip>, std::shared_ptr<av::io::IRead> > Reader;
        std::vector<Reader> _readers;
        std::shared_ptr<render::FontSystem> _fontSystem;
        std::shared_ptr<render::Render> _render;

        bool _running = true;
        std::chrono::steady_clock::time_point _startTime;
        otime::RationalTime _currentTime;
        Playback _playback = Playback::Stop;
        otime::RationalTime _playbackStartTime;
    };
}
