// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021 Darby Johnston
// All rights reserved.

#include <tlrApp/IApp.h>
#include <tlrApp/Util.h>

#include <tlrTimeline/Util.h>

#include <opentimelineio/clip.h>
#include <opentimelineio/timeline.h>
#include <opentimelineio/track.h>

#include <memory>
#include <string>
#include <vector>

namespace tlr
{
    //! Application options.
    struct Options
    {
        float windowScale = 1.F;
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
        void _readTimeline();

        void _fullscreenWindow();
        void _normalWindow();
        void _fullscreenCallback(bool);
        static void _frameBufferSizeCallback(GLFWwindow*, int, int);
        static void _windowContentScaleCallback(GLFWwindow*, float, float);
        static void _keyCallback(GLFWwindow*, int, int, int, int);
        void _printShortcutsHelp();

        void _tick();

        void _updateReaders();
        void _updateHUD();

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
        void _seekCallback(const otime::RationalTime&);

        std::string _input;
        Options _options;

        otio::SerializableObject::Retainer<otio::Timeline> _timeline;
        otio::SerializableObject::Retainer<otio::Track> _flattenedTimeline;
        otime::RationalTime _duration;
        imaging::Info _info;

        math::Vector2i _windowPos;

        typedef std::pair<otio::SerializableObject::Retainer<otio::Clip>, std::shared_ptr<av::io::IRead> > Reader;
        std::vector<Reader> _readers;

        bool _renderDirty = true;
        std::shared_ptr<imaging::Image> _currentImage;
        std::map<app::HUDElement, std::string> _hudLabels;

        bool _running = true;
        std::chrono::steady_clock::time_point _startTime;
        otime::RationalTime _currentTime;
        Playback _playback = Playback::Stop;
        otime::RationalTime _playbackStartTime;
    };
}
