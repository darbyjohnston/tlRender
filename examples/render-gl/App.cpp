// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2023 Darby Johnston
// All rights reserved.

#include "App.h"

#include <tlTimeline/GLRender.h>

#include <tlCore/Math.h>
#include <tlCore/StringFormat.h>
#include <tlCore/Time.h>

#if defined(TLRENDER_GL_DEBUG)
#include <tlGladDebug/gl.h>
#else // TLRENDER_GL_DEBUG
#include <tlGlad/gl.h>
#endif // TLRENDER_GL_DEBUG

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

#include <array>
#include <cmath>

namespace tl
{
    namespace examples
    {
        namespace render_gl
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
                    "render-gl",
                    "Example GLFW rendering application.",
                    {
                        app::CmdLineValueArg<std::string>::create(
                            _input,
                            "input",
                            "The input timeline.")
                    },
                {
                    app::CmdLineValueOption<std::string>::create(
                        _options.compareFileName,
                        { "-compare", "-b" },
                        "A/B comparison \"B\" file name."),
                    app::CmdLineValueOption<image::Size>::create(
                        _options.windowSize,
                        { "-windowSize", "-ws" },
                        "Window size.",
                        string::Format("{0}x{1}").arg(_options.windowSize.w).arg(_options.windowSize.h)),
                    app::CmdLineFlagOption::create(
                        _options.fullscreen,
                        { "-fullscreen", "-fs" },
                        "Enable full screen mode."),
                    app::CmdLineValueOption<bool>::create(
                        _options.hud,
                        { "-hud" },
                        "Enable the HUD (heads up display).",
                        string::Format("{0}").arg(_options.hud),
                        "0, 1"),
                    app::CmdLineValueOption<timeline::Playback>::create(
                        _options.playback,
                        { "-playback", "-p" },
                        "Playback mode.",
                        string::Format("{0}").arg(_options.playback),
                        string::join(timeline::getPlaybackLabels(), ", ")),
                    app::CmdLineValueOption<otime::RationalTime>::create(
                        _options.seek,
                        { "-seek" },
                        "Seek to the given time."),
                    app::CmdLineValueOption<otime::TimeRange>::create(
                        _options.inOutRange,
                        { "-inOutRange" },
                        "Set the in/out points range."),
                    app::CmdLineValueOption<std::string>::create(
                        _options.colorConfigOptions.fileName,
                        { "-colorConfig", "-cc" },
                        "Color configuration file name (e.g., config.ocio)."),
                    app::CmdLineValueOption<std::string>::create(
                        _options.colorConfigOptions.input,
                        { "-colorInput", "-ci" },
                        "Input color space."),
                    app::CmdLineValueOption<std::string>::create(
                        _options.colorConfigOptions.display,
                        { "-colorDisplay", "-cd" },
                        "Display color space."),
                    app::CmdLineValueOption<std::string>::create(
                        _options.colorConfigOptions.view,
                        { "-colorView", "-cv" },
                        "View color space."),
                    app::CmdLineValueOption<std::string>::create(
                        _options.lutOptions.fileName,
                        { "-lut" },
                        "LUT file name."),
                    app::CmdLineValueOption<timeline::LUTOrder>::create(
                        _options.lutOptions.order,
                        { "-lutOrder" },
                        "LUT operation order.",
                        string::Format("{0}").arg(_options.lutOptions.order),
                        string::join(timeline::getLUTOrderLabels(), ", "))
                });
            }

            App::App()
            {}

            App::~App()
            {
                _render.reset();
                if (_glfwWindow)
                {
                    glfwDestroyWindow(_glfwWindow);
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

            void App::run()
            {
                if (_exit != 0)
                {
                    return;
                }

                // Read the timelines.
                auto timeline = timeline::Timeline::create(_input, _context);
                auto player = timeline::Player::create(timeline, _context);
                _players.push_back(player);
                auto ioInfo = player->getIOInfo();
                if (!ioInfo.video.empty())
                {
                    _videoSizes.push_back(ioInfo.video[0].size);
                }
                _videoData.push_back(timeline::VideoData());
                if (!_options.compareFileName.empty())
                {
                    timeline = timeline::Timeline::create(_options.compareFileName, _context);
                    player = timeline::Player::create(timeline, _context);
                    player->setExternalTime(_players[0]);
                    _players.push_back(player);
                    ioInfo = player->getIOInfo();
                    if (!ioInfo.video.empty())
                    {
                        _videoSizes.push_back(ioInfo.video[0].size);
                    }
                    _videoData.push_back(timeline::VideoData());
                }

                // Create the window.
                glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
                glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
                glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GLFW_TRUE);
                glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
                glfwWindowHint(GLFW_DOUBLEBUFFER, GLFW_TRUE);
                _glfwWindow = glfwCreateWindow(
                    _options.windowSize.w,
                    _options.windowSize.h,
                    "render-gl",
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
                const int glMajor = glfwGetWindowAttrib(_glfwWindow, GLFW_CONTEXT_VERSION_MAJOR);
                const int glMinor = glfwGetWindowAttrib(_glfwWindow, GLFW_CONTEXT_VERSION_MINOR);
                const int glRevision = glfwGetWindowAttrib(_glfwWindow, GLFW_CONTEXT_REVISION);
                _log(string::Format("OpenGL version: {0}.{1}.{2}").arg(glMajor).arg(glMinor).arg(glRevision));
                glfwSetFramebufferSizeCallback(_glfwWindow, _frameBufferSizeCallback);
                glfwSetWindowContentScaleCallback(_glfwWindow, _windowContentScaleCallback);
                _setFullscreenWindow(_options.fullscreen);
                glfwSetKeyCallback(_glfwWindow, _keyCallback);
                glfwShowWindow(_glfwWindow);

                // Create the renderer.
                _render = timeline::GLRender::create(_context);

                // Print the shortcuts help.
                _printShortcutsHelp();

                // Start the main loop.
                _hud = _options.hud;
                if (time::isValid(_options.inOutRange))
                {
                    _players[0]->setInOutRange(_options.inOutRange);
                    _players[0]->seek(_options.inOutRange.start_time());
                }
                if (time::isValid(_options.seek))
                {
                    _players[0]->seek(_options.seek);
                }
                _players[0]->setPlayback(_options.playback);
                _startTime = std::chrono::steady_clock::now();
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

            void App::_setFullscreenWindow(bool value)
            {
                if (value == _fullscreen)
                    return;

                _fullscreen = value;

                if (_fullscreen)
                {
                    int width = 0;
                    int height = 0;
                    glfwGetWindowSize(_glfwWindow, &width, &height);
                    _windowSize.w = width;
                    _windowSize.h = height;

                    GLFWmonitor* glfwMonitor = glfwGetPrimaryMonitor();
                    const GLFWvidmode* glfwVidmode = glfwGetVideoMode(glfwMonitor);
                    glfwGetWindowPos(_glfwWindow, &_windowPos.x, &_windowPos.y);
                    glfwSetWindowMonitor(
                        _glfwWindow,
                        glfwMonitor,
                        0,
                        0,
                        glfwVidmode->width,
                        glfwVidmode->height,
                        glfwVidmode->refreshRate);
                }
                else
                {
                    GLFWmonitor* glfwMonitor = glfwGetPrimaryMonitor();
                    glfwSetWindowMonitor(
                        _glfwWindow,
                        NULL,
                        _windowPos.x,
                        _windowPos.y,
                        _windowSize.w,
                        _windowSize.h,
                        0);
                }
            }

            void App::_fullscreenCallback(bool value)
            {
                _setFullscreenWindow(value);
                _log(string::Format("Fullscreen: {0}").arg(_fullscreen));
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
                    switch (key)
                    {
                    case GLFW_KEY_ESCAPE:
                        app->exit();
                        break;
                    case GLFW_KEY_U:
                        app->_fullscreenCallback(!app->_fullscreen);
                        break;
                    case GLFW_KEY_H:
                        app->_hudCallback(!app->_hud);
                        break;
                    case GLFW_KEY_SPACE:
                        app->_playbackCallback(
                            timeline::Playback::Stop == app->_players[0]->observePlayback()->get() ?
                            timeline::Playback::Forward :
                            timeline::Playback::Stop);
                        break;
                    case GLFW_KEY_HOME:
                        app->_players[0]->start();
                        break;
                    case GLFW_KEY_END:
                        app->_players[0]->end();
                        break;
                    case GLFW_KEY_LEFT:
                        app->_players[0]->framePrev();
                        break;
                    case GLFW_KEY_RIGHT:
                        app->_players[0]->frameNext();
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
                    "    Home   - Go to the start time\n"
                    "    End    - Go to the end time\n"
                    "    Left   - Go to the previous frame\n"
                    "    Right  - Go to the next frame\n");
            }

            void App::_tick()
            {
                // Update.
                _context->tick();
                for (const auto& player : _players)
                {
                    player->tick();
                }
                for (size_t i = 0; i < _players.size(); ++i)
                {
                    const auto& videoData = _players[i]->observeCurrentVideo()->get();
                    if (!timeline::isTimeEqual(videoData, _videoData[i]))
                    {
                        _videoData[i] = videoData;
                        _renderDirty = true;
                    }
                }

                // Render the video.
                if (_renderDirty)
                {
                    _render->begin(
                        _frameBufferSize,
                        _options.colorConfigOptions,
                        _options.lutOptions);
                    _draw();
                    _render->end();
                    glfwSwapBuffers(_glfwWindow);
                    _renderDirty = false;
                }
                else
                {
                    time::sleep(std::chrono::milliseconds(5));
                }

                const auto now = std::chrono::steady_clock::now();
                const std::chrono::duration<float> diff = now - _startTime;
                const float v = (sinf(diff.count()) + 1.F) / 2.F;
                _compareOptions.wipeCenter.x = v;
                _compareOptions.overlay = v;
                _rotation = diff.count() * 2.F;
            }

            void App::_draw()
            {
                const int fontSize =
                    math::clamp(
                        ceilf(14 * _contentScale.y),
                        0.F,
                        static_cast<float>(std::numeric_limits<uint16_t>::max()));
                const int viewportSpacing = fontSize / 2;
                const math::Vector2i viewportSize(
                    (_frameBufferSize.w - viewportSpacing * 2) / 3,
                    (_frameBufferSize.h - viewportSpacing * 2) / 3);

                _compareOptions.mode = timeline::CompareMode::A;
                _drawViewport(
                    math::BBox2i(
                        0,
                        0,
                        viewportSize.x,
                        viewportSize.y),
                    fontSize,
                    _compareOptions,
                    0.F);
                _compareOptions.mode = timeline::CompareMode::A;
                _drawViewport(
                    math::BBox2i(
                        viewportSize.x + viewportSpacing,
                        0,
                        viewportSize.x,
                        viewportSize.y),
                    fontSize,
                    _compareOptions,
                    _rotation);
                _compareOptions.mode = timeline::CompareMode::B;
                _drawViewport(
                    math::BBox2i(
                        viewportSize.x * 2 + viewportSpacing * 2,
                        0,
                        viewportSize.x,
                        viewportSize.y),
                    fontSize,
                    _compareOptions,
                    _rotation);

                _compareOptions.mode = timeline::CompareMode::Wipe;
                _drawViewport(
                    math::BBox2i(
                        0,
                        viewportSize.y + viewportSpacing,
                        viewportSize.x,
                        viewportSize.y),
                    fontSize,
                    _compareOptions,
                    _rotation);
                _compareOptions.mode = timeline::CompareMode::Overlay;
                _drawViewport(
                    math::BBox2i(
                        viewportSize.x + viewportSpacing,
                        viewportSize.y + viewportSpacing,
                        viewportSize.x,
                        viewportSize.y),
                    fontSize,
                    _compareOptions,
                    _rotation);
                _compareOptions.mode = timeline::CompareMode::Difference;
                _drawViewport(
                    math::BBox2i(
                        viewportSize.x * 2 + viewportSpacing * 2,
                        viewportSize.y + viewportSpacing,
                        viewportSize.x,
                        viewportSize.y),
                    fontSize,
                    _compareOptions,
                    _rotation);

                _compareOptions.mode = timeline::CompareMode::Horizontal;
                _drawViewport(
                    math::BBox2i(
                        0,
                        viewportSize.y * 2 + viewportSpacing * 2,
                        viewportSize.x,
                        viewportSize.y),
                    fontSize,
                    _compareOptions,
                    _rotation);
                _compareOptions.mode = timeline::CompareMode::Vertical;
                _drawViewport(
                    math::BBox2i(
                        viewportSize.x + viewportSpacing,
                        viewportSize.y * 2 + viewportSpacing * 2,
                        viewportSize.x,
                        viewportSize.y),
                    fontSize,
                    _compareOptions,
                    _rotation);
                _compareOptions.mode = timeline::CompareMode::Tile;
                _drawViewport(
                    math::BBox2i(
                        viewportSize.x * 2 + viewportSpacing * 2,
                        viewportSize.y * 2 + viewportSpacing * 2,
                        viewportSize.x,
                        viewportSize.y),
                    fontSize,
                    _compareOptions,
                    _rotation);
            }

            void App::_drawViewport(
                const math::BBox2i& bbox,
                uint16_t fontSize,
                const timeline::CompareOptions& compareOptions,
                float rotation)
            {
                const math::Vector2i viewportSize = bbox.getSize();
                const float viewportAspect = viewportSize.y > 0 ?
                    (viewportSize.x / static_cast<float>(viewportSize.y)) :
                    1.F;
                const image::Size renderSize = timeline::getRenderSize(
                    compareOptions.mode,
                    _videoSizes);
                const float renderSizeAspect = renderSize.getAspect();
                image::Size transformSize;
                math::Vector2f transformOffset;
                if (renderSizeAspect > 1.F)
                {
                    transformSize.w = renderSize.w;
                    transformSize.h = renderSize.w / viewportAspect;
                    transformOffset.x = renderSize.w / 2.F;
                    transformOffset.y = renderSize.w / viewportAspect / 2.F;
                }
                else
                {
                    transformSize.w = renderSize.h * viewportAspect;
                    transformSize.h = renderSize.h;
                    transformOffset.x = renderSize.h * viewportAspect / 2.F;
                    transformOffset.y = renderSize.h / 2.F;
                }

                _render->setClipRectEnabled(true);
                _render->setViewport(bbox);
                _render->setClipRect(bbox);
                _render->clearViewport(image::Color4f(0.F, 0.F, 0.F));

                _render->setTransform(math::ortho(
                    0.F,
                    static_cast<float>(transformSize.w),
                    static_cast<float>(transformSize.h),
                    0.F,
                    -1.F,
                    1.F) *
                    math::translate(math::Vector3f(transformOffset.x, transformOffset.y, 0.F)) *
                    math::rotateZ(rotation) *
                    math::translate(math::Vector3f(-renderSize.w / 2, -renderSize.h / 2, 0.F)));
                _render->drawVideo(
                    _videoData,
                    timeline::getBBoxes(compareOptions.mode, _videoSizes),
                    {},
                    {},
                    compareOptions);

                if (_hud)
                {
                    _render->setTransform(math::ortho(
                        0.F,
                        static_cast<float>(viewportSize.x),
                        static_cast<float>(viewportSize.y),
                        0.F,
                        -1.F,
                        1.F));

                    image::FontInfo fontInfo;
                    fontInfo.size = fontSize;
                    auto fontSystem = _context->getSystem<image::FontSystem>();
                    auto fontMetrics = fontSystem->getMetrics(fontInfo);
                    std::string text = timeline::getLabel(compareOptions.mode);
                    math::Vector2i textSize = fontSystem->getSize(text, fontInfo);
                    _render->drawRect(
                        math::BBox2i(0, 0, viewportSize.x, fontMetrics.lineHeight),
                        image::Color4f(0.F, 0.F, 0.F, .7F));
                    _render->drawText(
                        fontSystem->getGlyphs(text, fontInfo),
                        math::Vector2i(fontSize / 5, fontMetrics.ascender),
                        image::Color4f(1.F, 1.F, 1.F));
                }

                _render->setClipRectEnabled(false);
            }

            void App::_hudCallback(bool value)
            {
                _hud = value;
                _renderDirty = true;
                _log(string::Format("HUD: {0}").arg(_hud));
            }

            void App::_playbackCallback(timeline::Playback value)
            {
                _players[0]->setPlayback(value);
                _log(string::Format("Playback: {0}").arg(_players[0]->observePlayback()->get()));
            }
        }
    }
}
