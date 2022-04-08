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

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

#include <array>

namespace tl
{
    namespace examples
    {
        namespace play_glfw
        {
            namespace
            {
                void glfwErrorCallback(int, const char* description)
                {
                    std::cerr << "GLFW ERROR: " << description << std::endl;
                }

                /*void APIENTRY glDebugOutput(
                    GLenum         source,
                    GLenum         type,
                    GLuint         id,
                    GLenum         severity,
                    GLsizei        length,
                    const GLchar * message,
                    const void *   userParam)
                {
                    switch (severity)
                    {
                    case GL_DEBUG_SEVERITY_HIGH_KHR:
                    case GL_DEBUG_SEVERITY_MEDIUM_KHR:
                        std::cerr << "DEBUG: " << message << std::endl;
                        break;
                    default: break;
                    }
                }*/
            }

            void App::_init(
                int argc,
                char* argv[],
                const std::shared_ptr<system::Context>& context)
            {
                IApp::_init(
                    argc,
                    argv,
                    context,
                    "play-glfw",
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
                if (_glfwWindow)
                {
                    glfwDestroyWindow(_glfwWindow);
                }
                glfwTerminate();
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

            void App::run()
            {
                if (_exit != 0)
                {
                    return;
                }

                // Read the timeline.
                timeline::Options options;
                auto audioSystem = _context->getSystem<audio::System>();
                const audio::Info audioInfo = audioSystem->getDefaultOutputInfo();
                options.ioOptions["ffmpeg/AudioChannelCount"] = string::Format("{0}").arg(audioInfo.channelCount);
                options.ioOptions["ffmpeg/AudioDataType"] = string::Format("{0}").arg(audioInfo.dataType);
                options.ioOptions["ffmpeg/AudioSampleRate"] = string::Format("{0}").arg(audioInfo.sampleRate);
                auto timeline = timeline::Timeline::create(_input, _context, options);
                _timelinePlayer = timeline::TimelinePlayer::create(timeline, _context);

                // Initialize GLFW.
                glfwSetErrorCallback(glfwErrorCallback);
                int glfwMajor = 0;
                int glfwMinor = 0;
                int glfwRevision = 0;
                glfwGetVersion(&glfwMajor, &glfwMinor, &glfwRevision);
                _log(string::Format("GLFW version: {0}.{1}.{2}").arg(glfwMajor).arg(glfwMinor).arg(glfwRevision));
                if (!glfwInit())
                {
                    throw std::runtime_error("Cannot initialize GLFW");
                }

                // Create the window.
                glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
                glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
                glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GLFW_TRUE);
                glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
                glfwWindowHint(GLFW_DOUBLEBUFFER, GLFW_TRUE);
                //glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GLFW_TRUE);
                _glfwWindow = glfwCreateWindow(
                    _options.windowSize.w,
                    _options.windowSize.h,
                    "play-glfw",
                    NULL,
                    NULL);
                if (!_glfwWindow)
                {
                    throw std::runtime_error("Cannot create window");
                }
                glfwSetWindowUserPointer(_glfwWindow, this);
                int width = 0;
                int height = 0;
                glfwGetFramebufferSize(_glfwWindow, &width, &height);
                _frameBufferSize.w = width;
                _frameBufferSize.h = height;
                glfwGetWindowContentScale(_glfwWindow, &_contentScale.x, &_contentScale.y);
                glfwMakeContextCurrent(_glfwWindow);
                if (!gladLoaderLoadGL())
                {
                    throw std::runtime_error("Cannot initialize GLAD");
                }
                /*GLint flags = 0;
                glGetIntegerv(GL_CONTEXT_FLAGS, &flags);
                if (flags & static_cast<GLint>(GL_CONTEXT_FLAG_DEBUG_BIT))
                {
                    glEnable(GL_DEBUG_OUTPUT);
                    glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
                    glDebugMessageCallback(glDebugOutput, context.get());
                    glDebugMessageControl(
                        static_cast<GLenum>(GL_DONT_CARE),
                        static_cast<GLenum>(GL_DONT_CARE),
                        static_cast<GLenum>(GL_DONT_CARE),
                        0,
                        nullptr,
                        GLFW_TRUE);
                }*/
                const int glMajor = glfwGetWindowAttrib(_glfwWindow, GLFW_CONTEXT_VERSION_MAJOR);
                const int glMinor = glfwGetWindowAttrib(_glfwWindow, GLFW_CONTEXT_VERSION_MINOR);
                const int glRevision = glfwGetWindowAttrib(_glfwWindow, GLFW_CONTEXT_REVISION);
                _log(string::Format("OpenGL version: {0}.{1}.{2}").arg(glMajor).arg(glMinor).arg(glRevision));
                glfwSetFramebufferSizeCallback(_glfwWindow, _frameBufferSizeCallback);
                glfwSetWindowContentScaleCallback(_glfwWindow, _windowContentScaleCallback);
                if (_options.fullScreen)
                {
                    _fullscreenWindow();
                }
                glfwSetKeyCallback(_glfwWindow, _keyCallback);
                glfwShowWindow(_glfwWindow);

                // Create the renderer.
                _fontSystem = imaging::FontSystem::create();
                _render = gl::Render::create(_context);

                // Print the shortcuts help.
                _printShortcutsHelp();

                // Start the main loop.
                if (_options.startPlayback)
                {
                    _timelinePlayer->setPlayback(timeline::Playback::Forward);
                }
                while (_running && !glfwWindowShouldClose(_glfwWindow))
                {
                    glfwPollEvents();
                    _tick();
                }
            }

            void App::exit()
            {
                _running = false;
            }

            void App::_fullscreenWindow()
            {
                _options.fullScreen = true;

                int width = 0;
                int height = 0;
                glfwGetWindowSize(_glfwWindow, &width, &height);
                _options.windowSize.w = width;
                _options.windowSize.h = height;

                GLFWmonitor* glfwMonitor = glfwGetPrimaryMonitor();
                const GLFWvidmode* glfwVidmode = glfwGetVideoMode(glfwMonitor);
                glfwGetWindowPos(_glfwWindow, &_windowPos.x, &_windowPos.y);
                glfwSetWindowMonitor(_glfwWindow, glfwMonitor, 0, 0, glfwVidmode->width, glfwVidmode->height, glfwVidmode->refreshRate);
            }

            void App::_normalWindow()
            {
                _options.fullScreen = false;

                GLFWmonitor* glfwMonitor = glfwGetPrimaryMonitor();
                glfwSetWindowMonitor(_glfwWindow, NULL, _windowPos.x, _windowPos.y, _options.windowSize.w, _options.windowSize.h, 0);
            }

            void App::_fullscreenCallback(bool value)
            {
                if (value)
                {
                    _fullscreenWindow();
                }
                else
                {
                    _normalWindow();
                }
                _log(string::Format("Fullscreen: {0}").arg(_options.fullScreen));
            }

            void App::_frameBufferSizeCallback(GLFWwindow* glfwWindow, int width, int height)
            {
                App* app = reinterpret_cast<App*>(glfwGetWindowUserPointer(glfwWindow));
                app->_frameBufferSize.w = width;
                app->_frameBufferSize.h = height;
                app->_renderDirty = true;
            }

            void App::_windowContentScaleCallback(GLFWwindow* glfwWindow, float x, float y)
            {
                App* app = reinterpret_cast<App*>(glfwGetWindowUserPointer(glfwWindow));
                app->_contentScale.x = x;
                app->_contentScale.y = y;
                app->_renderDirty = true;
            }

            void App::_keyCallback(GLFWwindow* glfwWindow, int key, int scanCode, int action, int mods)
            {
                if (GLFW_RELEASE == action || GLFW_REPEAT == action)
                {
                    App* app = reinterpret_cast<App*>(glfwGetWindowUserPointer(glfwWindow));
                    const otime::RationalTime& duration = app->_timelinePlayer->getDuration();
                    const otime::RationalTime& currentTime = app->_timelinePlayer->observeCurrentTime()->get();
                    switch (key)
                    {
                    case GLFW_KEY_ESCAPE:
                        app->exit();
                        break;
                    case GLFW_KEY_U:
                        app->_fullscreenCallback(!app->_options.fullScreen);
                        break;
                    case GLFW_KEY_H:
                        app->_hudCallback(!app->_options.hud);
                        break;
                    case GLFW_KEY_SPACE:
                        app->_playbackCallback(
                            timeline::Playback::Stop == app->_timelinePlayer->observePlayback()->get() ?
                            timeline::Playback::Forward :
                            timeline::Playback::Stop);
                        break;
                    case GLFW_KEY_L:
                        app->_loopPlaybackCallback(
                            timeline::Loop::Loop == app->_timelinePlayer->observeLoop()->get() ?
                            timeline::Loop::Once :
                            timeline::Loop::Loop);
                        break;
                    case GLFW_KEY_HOME:
                        app->_timelinePlayer->start();
                        break;
                    case GLFW_KEY_END:
                        app->_timelinePlayer->end();
                        break;
                    case GLFW_KEY_LEFT:
                        app->_timelinePlayer->framePrev();
                        break;
                    case GLFW_KEY_RIGHT:
                        app->_timelinePlayer->frameNext();
                        break;
                    }
                }
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

            void App::_tick()
            {
                // Update.
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
                    _render->setColorConfig(_options.colorConfig);
                    _render->begin(_frameBufferSize);
                    _drawVideo();
                    if (_options.hud)
                    {
                        _drawHUD();
                    }
                    _render->end();
                    glfwSwapBuffers(_glfwWindow);
                    _renderDirty = false;
                }
                else
                {
                    time::sleep(std::chrono::milliseconds(1));
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
                if (errorStatus != otio::ErrorStatus::OK)
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
