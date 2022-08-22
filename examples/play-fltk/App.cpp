// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2022 Darby Johnston
// All rights reserved.

#include "App.h"

#include <tlGL/Render.h>

#include <tlCore/AudioSystem.h>
#include <tlCore/Math.h>
#include <tlCore/StringFormat.h>
#include <tlCore/Time.h>

#include <tlGlad/gl.h>



#include <FL/Fl_Gl_Window.H>
#include <FL/Fl.H>

#include <array>


namespace tl
{
    class FLTK_Window : public Fl_Gl_Window
    {
    public:
        FLTK_Window( int W, int H, const char* L,
                     tl::examples::play_fltk::App* APP ) :
            Fl_Gl_Window( W, H, L )
            {
                mode( FL_RGB | FL_DOUBLE | FL_ALPHA | FL_STENCIL | FL_OPENGL3 );
                app = APP;
                resizable(this);
            };

        int handle( int event )
            {
                switch( event )
                {
                case FL_FOCUS:
                    return 1;
                case FL_ENTER:
                    return 1;
                case FL_LEAVE:
                    return 1;
                case FL_KEYBOARD:
                {
                    const otime::RationalTime& duration = app->_timelinePlayer->getDuration();
                    const otime::RationalTime& currentTime = app->_timelinePlayer->observeCurrentTime()->get();
                    int key = Fl::event_key();
                    switch (key)
                    {
                    case 'u':
                        if (!fullscreen_active())
                            fullscreen();
                        else
                            fullscreen_off();
                        return 1;
                        break;
                    case 'h':
                        app->_hudCallback(!app->_options.hud);
                        return 1;
                        break;
                    case ' ':
                        app->_playbackCallback(
                            timeline::Playback::Stop == app->_timelinePlayer->observePlayback()->get() ?
                            timeline::Playback::Forward :
                            timeline::Playback::Stop);
                        return 1;
                        break;
                    case 'l':
                        app->_loopPlaybackCallback(
                            timeline::Loop::Loop == app->_timelinePlayer->observeLoop()->get() ?
                            timeline::Loop::Once :
                            timeline::Loop::Loop);
                        return 1;
                        break;
                    case FL_Home:
                        app->_timelinePlayer->start();
                        return 1;
                        break;
                    case FL_End:
                        app->_timelinePlayer->end();
                        return 1;
                        break;
                    case FL_Left:
                        app->_timelinePlayer->framePrev();
                        return 1;
                        break;
                    case FL_Right:
                        app->_timelinePlayer->frameNext();
                        return 1;
                        break;
                    }
                    return 0;
                }
                }
                return Fl_Gl_Window::handle( event );
            }

        void draw()
            {

                if ( !valid() )
                {
                    gladLoaderLoadGL();
                    valid(1);
                }

                app->draw();
                Fl_Gl_Window::draw();
            }

        void resize( int X, int Y, int W, int H )
            {
                app->_frameBufferSize.w = W;
                app->_frameBufferSize.h = H;
                app->_renderDirty = true;
                return Fl_Gl_Window::resize( X, Y, W, H );
            }

        examples::play_fltk::App* app;
    };

    namespace examples
    {
        namespace play_fltk
        {

            void App::_init(
                int argc,
                char* argv[],
                const std::shared_ptr<system::Context>& context)
            {
                IApp::_init(
                    argc,
                    argv,
                    context,
                    "play-fltk",
                    "Play an editorial timeline.",
                    {
                        app::CmdLineValueArg<std::string>::create(
                            _input,
                            "input",
                            "The input timeline.")
                    },
            {
                app::CmdLineValueOption<imaging::Size>::create(
                    _options.windowSize,
                    { "-windowSize", "-ws" },
                    "Window size.",
                    string::Format("{0}x{1}").arg(_options.windowSize.w).arg(_options.windowSize.h)),
                app::CmdLineFlagOption::create(
                    _options.fullScreen,
                    { "-fullScreen", "-fs" },
                    "Enable full screen mode."),
                app::CmdLineValueOption<bool>::create(
                    _options.hud,
                    { "-hud" },
                    "Enable the HUD (heads up display).",
                    string::Format("{0}").arg(_options.hud),
                    "0, 1"),
                app::CmdLineValueOption<bool>::create(
                    _options.startPlayback,
                    { "-startPlayback", "-sp" },
                    "Automatically start playback.",
                    string::Format("{0}").arg(_options.startPlayback),
                    "0, 1"),
                app::CmdLineValueOption<bool>::create(
                    _options.loopPlayback,
                    { "-loopPlayback", "-lp" },
                    "Loop playback.",
                    string::Format("{0}").arg(_options.loopPlayback),
                    "0, 1"),
                app::CmdLineValueOption<std::string>::create(
                    _options.colorConfig.fileName,
                    { "-colorConfig", "-cc" },
                    "Color configuration file name (e.g., config.ocio)."),
                app::CmdLineValueOption<std::string>::create(
                    _options.colorConfig.input,
                    { "-colorInput", "-ci" },
                    "Input color space."),
                app::CmdLineValueOption<std::string>::create(
                    _options.colorConfig.display,
                    { "-colorDisplay", "-cd" },
                    "Display color space."),
                app::CmdLineValueOption<std::string>::create(
                    _options.colorConfig.view,
                    { "-colorView", "-cv" },
                    "View color space.")
            });
            }

            App::App()
            {}

            App::~App()
            {
                _render.reset();
                _fontSystem.reset();
                if (_fltkWindow)
                {
                    delete _fltkWindow;
                }
            }

            std::shared_ptr<App> App::create(
                int argc,
                char* argv[],
                const std::shared_ptr<system::Context>& context)
            {
                auto out = std::shared_ptr<App>(new App);
                out->_init(argc, argv, context);
                return out;
            }

            int App::run()
            {

                // Read the timeline.
                timeline::Options options;
                auto audioSystem = _context->getSystem<audio::System>();
                const audio::Info audioInfo = audioSystem->getDefaultOutputInfo();
                options.ioOptions["ffmpeg/AudioChannelCount"] = string::Format("{0}").arg(audioInfo.channelCount);
                options.ioOptions["ffmpeg/AudioDataType"] = string::Format("{0}").arg(audioInfo.dataType);
                options.ioOptions["ffmpeg/AudioSampleRate"] = string::Format("{0}").arg(audioInfo.sampleRate);
                auto timeline = timeline::Timeline::create(_input, _context, options);
                _timelinePlayer = timeline::TimelinePlayer::create(timeline, _context);

                // Initialize FLTK.

                // Create the window.
                _fltkWindow = new FLTK_Window( _options.windowSize.w,
                                               _options.windowSize.h,
                                               "play-fltk", this );
                if (!_fltkWindow)
                {
                    throw std::runtime_error("Cannot create window");
                }
                if (_options.fullScreen)
                {
                    _fltkWindow->fullscreen();
                }

                _contentScale.y = _fltkWindow->pixels_per_unit();

                // Create the renderer.
                _fontSystem = imaging::FontSystem::create(_context);
                _render = gl::Render::create(_context);

                _fltkWindow->show();
                Fl::check();
                _frameBufferSize.w = _fltkWindow->w();
                _frameBufferSize.h = _fltkWindow->h();
                // Print the shortcuts help.
                _printShortcutsHelp();

                // Start the main loop.
                if (_options.startPlayback)
                {
                    _timelinePlayer->setPlayback(timeline::Playback::Forward);
                }
                while ( _running )
                {
                    _tick();
                }
                return 0;
            }



            void App::_printShortcutsHelp()
            {
                _print(
                    "\n"
                    "Keyboard shortcuts:\n"
                    "\n"
                    "    Escape - Exit\n"
                    "    U      - Fullscreen mode\n"
                    "    H      - HUD enabled\n"
                    "    Space  - Start/stop playback\n"
                    "    L      - Loop playback\n"
                    "    Home   - Go to the start time\n"
                    "    End    - Go to the end time\n"
                    "    Left   - Go to the previous frame\n"
                    "    Right  - Go to the next frame\n");
            }

            void App::draw()
            {
                _render->setColorConfig(_options.colorConfig);
                _render->begin(_frameBufferSize);
                _drawVideo();
                if (_options.hud)
                {
                    _drawHUD();
                }
                _render->end();
            }

            void App::_tick()
            {
                // Update
                _context->tick();
                _timelinePlayer->tick();
                const auto& videoData = _timelinePlayer->observeVideo()->get();
                if (!timeline::isTimeEqual(videoData, _videoData))
                {
                    _videoData = videoData;
                    _renderDirty = true;
                }
                _hudUpdate();


                // Render the video.
                if (_renderDirty)
                {
                    _fltkWindow->redraw();
                    _running = Fl::check();
                    _renderDirty = false;
                }
            }

            void App::_hudUpdate()
            {
                std::map<HUDElement, std::string> hudLabels;

                // Input file name.
                hudLabels[HUDElement::UpperLeft] = "Input: " + _input;

                // Current time.
                otime::ErrorStatus errorStatus;
                const std::string label = _timelinePlayer->observeCurrentTime()->get().to_timecode(&errorStatus);
                if (otime::is_error(errorStatus))
                {
                    throw std::runtime_error(errorStatus.details);
                }
                hudLabels[HUDElement::LowerLeft] = "Time: " + label;

                // Cache percentage.
                const float cachePercentage = _timelinePlayer->observeCachePercentage()->get();
                hudLabels[HUDElement::UpperRight] = string::Format("Cache: {0}%").arg(cachePercentage, 0, 3);

                // Speed.
                hudLabels[HUDElement::LowerRight] = string::Format("Speed: {0}").arg(_timelinePlayer->getDuration().rate(), 2);

                if (hudLabels != _hudLabels)
                {
                    _hudLabels = hudLabels;
                    _renderDirty = true;
                }
            }

            void App::_drawVideo()
            {
                _render->drawVideo(
                    { _videoData },
                    { math::BBox2i(0, 0, _frameBufferSize.w, _frameBufferSize.h) });
            }

            void App::_drawHUD()
            {
                const uint16_t fontSize =
                    math::clamp(
                        ceilf(14 * _contentScale.y),
                        0.F,
                        static_cast<float>(std::numeric_limits<uint16_t>::max()));

                auto i = _hudLabels.find(HUDElement::UpperLeft);
                if (i != _hudLabels.end())
                {
                    drawHUDLabel(
                        _render,
                        _fontSystem,
                        _frameBufferSize,
                        i->second,
                        imaging::FontFamily::NotoSans,
                        fontSize,
                        HUDElement::UpperLeft);
                }

                i = _hudLabels.find(HUDElement::LowerLeft);
                if (i != _hudLabels.end())
                {
                    drawHUDLabel(
                        _render,
                        _fontSystem,
                        _frameBufferSize,
                        i->second,
                        imaging::FontFamily::NotoMono,
                        fontSize,
                        HUDElement::LowerLeft);
                }

                i = _hudLabels.find(HUDElement::UpperRight);
                if (i != _hudLabels.end())
                {
                    drawHUDLabel(
                        _render,
                        _fontSystem,
                        _frameBufferSize,
                        i->second,
                        imaging::FontFamily::NotoMono,
                        fontSize,
                        HUDElement::UpperRight);
                }

                i = _hudLabels.find(HUDElement::LowerRight);
                if (i != _hudLabels.end())
                {
                    drawHUDLabel(
                        _render,
                        _fontSystem,
                        _frameBufferSize,
                        i->second,
                        imaging::FontFamily::NotoMono,
                        fontSize,
                        HUDElement::LowerRight);
                }
            }

            void App::_hudCallback(bool value)
            {
                _options.hud = value;
                _renderDirty = true;
                _log(string::Format("HUD: {0}").arg(_options.hud));
            }

            void App::_playbackCallback(timeline::Playback value)
            {
                _timelinePlayer->setPlayback(value);
                _log(string::Format("Playback: {0}").arg(_timelinePlayer->observePlayback()->get()));
            }

            void App::_loopPlaybackCallback(timeline::Loop value)
            {
                _timelinePlayer->setLoop(value);
                _log(string::Format("Loop playback: {0}").arg(_timelinePlayer->observeLoop()->get()));
            }
        }
    }
}
