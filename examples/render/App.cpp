// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2024 Darby Johnston
// All rights reserved.

#include "App.h"

#include <tlTimelineGL/Render.h>

#include <dtk/gl/GL.h>
#include <dtk/gl/Window.h>
#include <dtk/core/CmdLine.h>
#include <dtk/core/Context.h>
#include <dtk/core/Format.h>
#include <dtk/core/Math.h>
#include <dtk/core/String.h>
#include <dtk/core/Time.h>

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

#include <array>
#include <cmath>

namespace tl
{
    namespace examples
    {
        namespace render
        {
            void App::_init(
                const std::shared_ptr<dtk::Context>& context,
                std::vector<std::string>& argv)
            {
                IApp::_init(
                    context,
                    argv,
                    "render",
                    "Example rendering application.",
                    {
                        dtk::CmdLineValueArg<std::string>::create(
                            _input,
                            "input",
                            "The input timeline.")
                    },
                {
                    dtk::CmdLineValueOption<std::string>::create(
                        _options.compareFileName,
                        { "-compare", "-b" },
                        "A/B comparison \"B\" file name."),
                    dtk::CmdLineValueOption<dtk::Size2I>::create(
                        _options.windowSize,
                        { "-windowSize", "-ws" },
                        "Window size.",
                        dtk::Format("{0}x{1}").arg(_options.windowSize.w).arg(_options.windowSize.h)),
                    dtk::CmdLineFlagOption::create(
                        _options.fullscreen,
                        { "-fullscreen", "-fs" },
                        "Enable full screen mode."),
                    dtk::CmdLineValueOption<bool>::create(
                        _options.hud,
                        { "-hud" },
                        "Enable the HUD (heads up display).",
                        dtk::Format("{0}").arg(_options.hud),
                        "0, 1"),
                    dtk::CmdLineValueOption<timeline::Playback>::create(
                        _options.playback,
                        { "-playback", "-p" },
                        "Playback mode.",
                        dtk::Format("{0}").arg(_options.playback),
                        dtk::join(timeline::getPlaybackLabels(), ", ")),
                    dtk::CmdLineValueOption<OTIO_NS::RationalTime>::create(
                        _options.seek,
                        { "-seek" },
                        "Seek to the given time."),
                    dtk::CmdLineValueOption<OTIO_NS::TimeRange>::create(
                        _options.inOutRange,
                        { "-inOutRange" },
                        "Set the in/out points range."),
                    dtk::CmdLineValueOption<std::string>::create(
                        _options.ocioOptions.fileName,
                        { "-ocio" },
                        "OpenColorIO configuration file name (e.g., config.ocio)."),
                    dtk::CmdLineValueOption<std::string>::create(
                        _options.ocioOptions.input,
                        { "-ocioInput" },
                        "OpenColorIO input name."),
                    dtk::CmdLineValueOption<std::string>::create(
                        _options.ocioOptions.display,
                        { "-ocioDisplay" },
                        "OpenColorIO display name."),
                    dtk::CmdLineValueOption<std::string>::create(
                        _options.ocioOptions.view,
                        { "-ocioView" },
                        "OpenColorIO view name."),
                    dtk::CmdLineValueOption<std::string>::create(
                        _options.ocioOptions.look,
                        { "-ocioLook" },
                        "OpenColorIO look name."),
                    dtk::CmdLineValueOption<std::string>::create(
                        _options.lutOptions.fileName,
                        { "-lut" },
                        "LUT file name."),
                    dtk::CmdLineValueOption<timeline::LUTOrder>::create(
                        _options.lutOptions.order,
                        { "-lutOrder" },
                        "LUT operation order.",
                        dtk::Format("{0}").arg(_options.lutOptions.order),
                        dtk::join(timeline::getLUTOrderLabels(), ", "))
                });
            }

            App::App()
            {}

            App::~App()
            {}

            std::shared_ptr<App> App::create(
                const std::shared_ptr<dtk::Context>& context,
                std::vector<std::string>& argv)
            {
                auto out = std::shared_ptr<App>(new App);
                out->_init(context, argv);
                return out;
            }

            void App::run()
            {
                // Read the timelines.
                auto timeline = timeline::Timeline::create(_context, _input);
                _player = timeline::Player::create(_context, timeline);
                std::vector<std::shared_ptr <timeline::Timeline> > compare;
                if (!_options.compareFileName.empty())
                {
                    compare.push_back(timeline::Timeline::create(_context, _options.compareFileName));
                }
                _player->setCompare(compare);
                _videoDataObserver = dtk::ListObserver<timeline::VideoData>::create(
                    _player->observeCurrentVideo(),
                    [this](const std::vector<timeline::VideoData>& value)
                    {
                        _videoData = value;
                        _renderDirty = true;
                    });

                // Create the window.
                _window = dtk::gl::Window::create(
                    _context,
                    "render",
                    _options.windowSize);
                _frameBufferSize = _window->getFrameBufferSize();
                _contentScale = _window->getContentScale();
                _window->setFullScreen(_options.fullscreen);
                _window->setFrameBufferSizeCallback(
                    [this](const dtk::Size2I& value)
                    {
                        _frameBufferSize = value;
                        _renderDirty = true;
                    });
                _window->setContentScaleCallback(
                    [this](const dtk::V2F& value)
                    {
                        _contentScale = value;
                        _renderDirty = true;
                    });
                _window->setKeyCallback(
                    [this](int key, int scanCode, int action, int mods)
                    {
                        _keyCallback(key, scanCode, action, mods);
                    });
                _window->setCloseCallback(
                    [this]
                    {
                        _running = false;
                    });

                // Create the renderer.
                _render = timeline_gl::Render::create(_context);

                // Print the shortcuts help.
                _printShortcutsHelp();

                // Start the main loop.
                _hud = _options.hud;
                if (time::isValid(_options.inOutRange))
                {
                    _player->setInOutRange(_options.inOutRange);
                    _player->seek(_options.inOutRange.start_time());
                }
                if (time::isValid(_options.seek))
                {
                    _player->seek(_options.seek);
                }
                _player->setPlayback(_options.playback);
                _startTime = std::chrono::steady_clock::now();
                while (_running)
                {
                    glfwPollEvents();
                    _tick();
                }
            }

            void App::_keyCallback(int key, int scanCode, int action, int mods)
            {
                if (GLFW_RELEASE == action || GLFW_REPEAT == action)
                {
                    switch (key)
                    {
                    case GLFW_KEY_ESCAPE:
                        _running = false;
                        break;
                    case GLFW_KEY_U:
                        _window->setFullScreen(!_window->isFullScreen());
                        break;
                    case GLFW_KEY_H:
                        _hudCallback(!_hud);
                        break;
                    case GLFW_KEY_SPACE:
                        _playbackCallback(
                            timeline::Playback::Stop == _player->observePlayback()->get() ?
                            timeline::Playback::Forward :
                            timeline::Playback::Stop);
                        break;
                    case GLFW_KEY_HOME:
                        _player->gotoStart();
                        break;
                    case GLFW_KEY_END:
                        _player->gotoEnd();
                        break;
                    case GLFW_KEY_LEFT:
                        _player->framePrev();
                        break;
                    case GLFW_KEY_RIGHT:
                        _player->frameNext();
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
                const auto t0 = std::chrono::steady_clock::now();

                // Update.
                _context->tick();
                _player->tick();

                // Render the video.
                if (_renderDirty)
                {
                    _render->begin(_frameBufferSize);
                    _render->setOCIOOptions(_options.ocioOptions);
                    _render->setLUTOptions(_options.lutOptions);
                    _draw();
                    _render->end();
                    _window->swap();
                    _renderDirty = false;
                }

                // Update the animation.
                const auto t1 = std::chrono::steady_clock::now();
                const std::chrono::duration<float> diff = t1 - _startTime;
                const float v = (sinf(diff.count()) + 1.F) / 2.F;
                _compareOptions.wipeCenter.x = v;
                _compareOptions.overlay = v;
                _rotation = diff.count() * 2.F;

                // Sleep.
                dtk::sleep(std::chrono::milliseconds(5), t0, t1);
            }

            void App::_draw()
            {
                const int fontSize =
                    dtk::clamp(
                        ceilf(14 * _contentScale.y),
                        0.F,
                        static_cast<float>(std::numeric_limits<uint16_t>::max()));
                const int viewportSpacing = fontSize / 2;
                const dtk::V2I viewportSize(
                    (_frameBufferSize.w - viewportSpacing * 2) / 3,
                    (_frameBufferSize.h - viewportSpacing * 2) / 3);

                _compareOptions.compare = timeline::Compare::A;
                _drawViewport(
                    dtk::Box2I(
                        0,
                        0,
                        viewportSize.x,
                        viewportSize.y),
                    fontSize,
                    _compareOptions,
                    0.F);
                _compareOptions.compare = timeline::Compare::A;
                _drawViewport(
                    dtk::Box2I(
                        viewportSize.x + viewportSpacing,
                        0,
                        viewportSize.x,
                        viewportSize.y),
                    fontSize,
                    _compareOptions,
                    _rotation);
                _compareOptions.compare = timeline::Compare::B;
                _drawViewport(
                    dtk::Box2I(
                        viewportSize.x * 2 + viewportSpacing * 2,
                        0,
                        viewportSize.x,
                        viewportSize.y),
                    fontSize,
                    _compareOptions,
                    _rotation);

                _compareOptions.compare = timeline::Compare::Wipe;
                _drawViewport(
                    dtk::Box2I(
                        0,
                        viewportSize.y + viewportSpacing,
                        viewportSize.x,
                        viewportSize.y),
                    fontSize,
                    _compareOptions,
                    _rotation);
                _compareOptions.compare = timeline::Compare::Overlay;
                _drawViewport(
                    dtk::Box2I(
                        viewportSize.x + viewportSpacing,
                        viewportSize.y + viewportSpacing,
                        viewportSize.x,
                        viewportSize.y),
                    fontSize,
                    _compareOptions,
                    _rotation);
                _compareOptions.compare = timeline::Compare::Difference;
                _drawViewport(
                    dtk::Box2I(
                        viewportSize.x * 2 + viewportSpacing * 2,
                        viewportSize.y + viewportSpacing,
                        viewportSize.x,
                        viewportSize.y),
                    fontSize,
                    _compareOptions,
                    _rotation);

                _compareOptions.compare = timeline::Compare::Horizontal;
                _drawViewport(
                    dtk::Box2I(
                        0,
                        viewportSize.y * 2 + viewportSpacing * 2,
                        viewportSize.x,
                        viewportSize.y),
                    fontSize,
                    _compareOptions,
                    _rotation);
                _compareOptions.compare = timeline::Compare::Vertical;
                _drawViewport(
                    dtk::Box2I(
                        viewportSize.x + viewportSpacing,
                        viewportSize.y * 2 + viewportSpacing * 2,
                        viewportSize.x,
                        viewportSize.y),
                    fontSize,
                    _compareOptions,
                    _rotation);
                _compareOptions.compare = timeline::Compare::Tile;
                _drawViewport(
                    dtk::Box2I(
                        viewportSize.x * 2 + viewportSpacing * 2,
                        viewportSize.y * 2 + viewportSpacing * 2,
                        viewportSize.x,
                        viewportSize.y),
                    fontSize,
                    _compareOptions,
                    _rotation);
            }

            void App::_drawViewport(
                const dtk::Box2I& box,
                uint16_t fontSize,
                const timeline::CompareOptions& compareOptions,
                float rotation)
            {
                const dtk::Size2I viewportSize = box.size();
                const float viewportAspect = dtk::aspectRatio(viewportSize);
                const dtk::Size2I renderSize = timeline::getRenderSize(
                    compareOptions.compare,
                    _videoData);
                const float renderSizeAspect = dtk::aspectRatio(renderSize);
                dtk::Size2I transformSize;
                dtk::V2F transformOffset;
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
                _render->setViewport(box);
                _render->setClipRect(box);
                _render->clearViewport(dtk::Color4F(0.F, 0.F, 0.F));

                _render->setTransform(dtk::ortho(
                    0.F,
                    static_cast<float>(transformSize.w),
                    static_cast<float>(transformSize.h),
                    0.F,
                    -1.F,
                    1.F) *
                    dtk::translate(dtk::V3F(transformOffset.x, transformOffset.y, 0.F)) *
                    dtk::rotateZ(rotation) *
                    dtk::translate(dtk::V3F(-renderSize.w / 2, -renderSize.h / 2, 0.F)));
                _render->drawVideo(
                    _videoData,
                    timeline::getBoxes(compareOptions.compare, _videoData),
                    {},
                    {},
                    compareOptions);

                if (_hud)
                {
                    _render->setTransform(dtk::ortho(
                        0.F,
                        static_cast<float>(viewportSize.w),
                        static_cast<float>(viewportSize.h),
                        0.F,
                        -1.F,
                        1.F));

                    dtk::FontInfo fontInfo;
                    fontInfo.size = fontSize;
                    auto fontSystem = _context->getSystem<dtk::FontSystem>();
                    auto fontMetrics = fontSystem->getMetrics(fontInfo);
                    std::string text = timeline::getLabel(compareOptions.compare);
                    dtk::Size2I textSize = fontSystem->getSize(text, fontInfo);
                    _render->drawRect(
                        dtk::Box2I(0, 0, viewportSize.w, fontMetrics.lineHeight),
                        dtk::Color4F(0.F, 0.F, 0.F, .7F));
                    _render->drawText(
                        fontSystem->getGlyphs(text, fontInfo),
                        fontMetrics,
                        dtk::V2I(fontSize / 5, 0),
                        dtk::Color4F(1.F, 1.F, 1.F));
                }

                _render->setClipRectEnabled(false);
            }

            void App::_hudCallback(bool value)
            {
                _hud = value;
                _renderDirty = true;
                _context->getLogSystem()->print("render", dtk::Format("HUD: {0}").arg(_hud));
            }

            void App::_playbackCallback(timeline::Playback value)
            {
                _player->setPlayback(value);
                _context->getLogSystem()->print("render", dtk::Format("Playback: {0}").arg(_player->observePlayback()->get()));
            }
        }
    }
}
