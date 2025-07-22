// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2024 Darby Johnston
// All rights reserved.

#include "App.h"

#include <tlTimelineGL/Render.h>

#include <feather-tk/gl/GL.h>
#include <feather-tk/gl/Window.h>
#include <feather-tk/core/Context.h>
#include <feather-tk/core/Format.h>
#include <feather-tk/core/Math.h>
#include <feather-tk/core/String.h>
#include <feather-tk/core/Time.h>

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
                const std::shared_ptr<feather_tk::Context>& context,
                std::vector<std::string>& argv)
            {
                _cmdLine.input = feather_tk::CmdLineValueArg<std::string>::create(
                    "input",
                    "The input timeline.");
                _cmdLine.compareFileName = feather_tk::CmdLineValueOption<std::string>::create(
                    { "-compare", "-b" },
                    "A/B comparison \"B\" file name.",
                    "Compare");
                _cmdLine.windowSize = feather_tk::CmdLineValueOption<feather_tk::Size2I>::create(
                    { "-windowSize", "-ws" },
                    "Window size.",
                    "Window",
                    feather_tk::Size2I(1920, 1080));
                _cmdLine.fullscreen = feather_tk::CmdLineFlagOption::create(
                    { "-fullscreen", "-fs" },
                    "Enable full screen mode.",
                    "Window");
                _cmdLine.hud = feather_tk::CmdLineFlagOption::create(
                    { "-hud" },
                    "Enable the HUD (heads up display).",
                    "View");
                _cmdLine.playback = feather_tk::CmdLineValueOption<timeline::Playback>::create(
                    { "-playback", "-p" },
                    "Playback mode.",
                    "Playback",
                    timeline::Playback::Forward,
                    feather_tk::join(timeline::getPlaybackLabels(), ", "));
                _cmdLine.seek = feather_tk::CmdLineValueOption<OTIO_NS::RationalTime>::create(
                    { "-seek" },
                    "Seek to the given time.",
                    "Playback");
                _cmdLine.inOutRange = feather_tk::CmdLineValueOption<OTIO_NS::TimeRange>::create(
                    { "-inOutRange" },
                    "Set the in/out points range.",
                    "Playback");
                _cmdLine.ocioFileName = feather_tk::CmdLineValueOption<std::string>::create(
                    { "-ocio" },
                    "OCIO configuration file name (e.g., config.ocio).",
                    "Color");
                _cmdLine.ocioInput = feather_tk::CmdLineValueOption<std::string>::create(
                    { "-ocioInput" },
                    "OCIO input name.",
                    "Color");
                _cmdLine.ocioDisplay = feather_tk::CmdLineValueOption<std::string>::create(
                    { "-ocioDisplay" },
                    "OCIO display name.",
                    "Color");
                _cmdLine.ocioView = feather_tk::CmdLineValueOption<std::string>::create(
                    { "-ocioView" },
                    "OCIO view name.",
                    "Color");
                _cmdLine.ocioLook = feather_tk::CmdLineValueOption<std::string>::create(
                    { "-ocioLook" },
                    "OCIO look name.",
                    "Color");
                _cmdLine.lutFileName = feather_tk::CmdLineValueOption<std::string>::create(
                    { "-lut" },
                    "LUT file name.",
                    "Color");
                _cmdLine.lutOrder = feather_tk::CmdLineValueOption<timeline::LUTOrder>::create(
                    { "-lutOrder" },
                    "LUT operation order.",
                    "Color",
                    std::optional<timeline::LUTOrder>(),
                    feather_tk::join(timeline::getLUTOrderLabels(), ", "));

                IApp::_init(
                    context,
                    argv,
                    "render",
                    "Example rendering application.",
                    { _cmdLine.input },
                {
                    _cmdLine.compareFileName,
                    _cmdLine.windowSize,
                    _cmdLine.fullscreen,
                    _cmdLine.hud,
                    _cmdLine.playback,
                    _cmdLine.seek,
                    _cmdLine.inOutRange,
                    _cmdLine.ocioFileName,
                    _cmdLine.ocioInput,
                    _cmdLine.ocioDisplay,
                    _cmdLine.ocioView,
                    _cmdLine.ocioLook,
                    _cmdLine.lutFileName,
                    _cmdLine.lutOrder
                });
            }

            App::App()
            {}

            App::~App()
            {}

            std::shared_ptr<App> App::create(
                const std::shared_ptr<feather_tk::Context>& context,
                std::vector<std::string>& argv)
            {
                auto out = std::shared_ptr<App>(new App);
                out->_init(context, argv);
                return out;
            }

            void App::run()
            {
                // Read the timelines.
                auto timeline = timeline::Timeline::create(
                    _context,
                    _cmdLine.input->getValue());
                _player = timeline::Player::create(_context, timeline);
                std::vector<std::shared_ptr <timeline::Timeline> > compare;
                if (_cmdLine.compareFileName->hasValue())
                {
                    compare.push_back(timeline::Timeline::create(
                        _context,
                        _cmdLine.compareFileName->getValue()));
                }
                _player->setCompare(compare);
                _videoDataObserver = feather_tk::ListObserver<timeline::VideoData>::create(
                    _player->observeCurrentVideo(),
                    [this](const std::vector<timeline::VideoData>& value)
                    {
                        _videoData = value;
                        _renderDirty = true;
                    });

                // Create the window.
                _window = feather_tk::gl::Window::create(
                    _context,
                    "render",
                    _cmdLine.windowSize->getValue());
                _frameBufferSize = _window->getFrameBufferSize();
                _contentScale = _window->getContentScale();
                _window->setFullScreen(_cmdLine.fullscreen->found());
                _window->setFrameBufferSizeCallback(
                    [this](const feather_tk::Size2I& value)
                    {
                        _frameBufferSize = value;
                        _renderDirty = true;
                    });
                _window->setContentScaleCallback(
                    [this](const feather_tk::V2F& value)
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

                // Set options.
                _hud = _cmdLine.hud->found();
                if (_cmdLine.inOutRange->hasValue())
                {
                    const OTIO_NS::TimeRange timeRange = _cmdLine.inOutRange->getValue();
                    _player->setInOutRange(timeRange);
                    _player->seek(timeRange.start_time());
                }
                if (_cmdLine.seek->hasValue())
                {
                    _player->seek(_cmdLine.seek->getValue());
                }
                _player->setPlayback(_cmdLine.playback->getValue());
                if (_cmdLine.ocioFileName->hasValue())
                {
                    _ocioOptions.fileName = _cmdLine.ocioFileName->getValue();
                }
                if (_cmdLine.ocioInput->hasValue())
                {
                    _ocioOptions.input = _cmdLine.ocioInput->getValue();
                }
                if (_cmdLine.ocioDisplay->hasValue())
                {
                    _ocioOptions.display = _cmdLine.ocioDisplay->getValue();
                }
                if (_cmdLine.ocioView->hasValue())
                {
                    _ocioOptions.view = _cmdLine.ocioView->getValue();
                }
                if (_cmdLine.ocioLook->hasValue())
                {
                    _ocioOptions.look = _cmdLine.ocioLook->getValue();
                }
                if (_cmdLine.lutFileName->hasValue())
                {
                    _lutOptions.fileName = _cmdLine.lutFileName->getValue();
                }
                if (_cmdLine.lutOrder->hasValue())
                {
                    _lutOptions.order = _cmdLine.lutOrder->getValue();
                }

                // Start the main loop.
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
                    _render->setOCIOOptions(_ocioOptions);
                    _render->setLUTOptions(_lutOptions);
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
                feather_tk::sleep(std::chrono::milliseconds(5), t0, t1);
            }

            void App::_draw()
            {
                const int fontSize =
                    feather_tk::clamp(
                        ceilf(14 * _contentScale.y),
                        0.F,
                        static_cast<float>(std::numeric_limits<uint16_t>::max()));
                const int viewportSpacing = fontSize / 2;
                const feather_tk::V2I viewportSize(
                    (_frameBufferSize.w - viewportSpacing * 2) / 3,
                    (_frameBufferSize.h - viewportSpacing * 2) / 3);

                _compareOptions.compare = timeline::Compare::A;
                _drawViewport(
                    feather_tk::Box2I(
                        0,
                        0,
                        viewportSize.x,
                        viewportSize.y),
                    fontSize,
                    _compareOptions,
                    0.F);
                _compareOptions.compare = timeline::Compare::A;
                _drawViewport(
                    feather_tk::Box2I(
                        viewportSize.x + viewportSpacing,
                        0,
                        viewportSize.x,
                        viewportSize.y),
                    fontSize,
                    _compareOptions,
                    _rotation);
                _compareOptions.compare = timeline::Compare::B;
                _drawViewport(
                    feather_tk::Box2I(
                        viewportSize.x * 2 + viewportSpacing * 2,
                        0,
                        viewportSize.x,
                        viewportSize.y),
                    fontSize,
                    _compareOptions,
                    _rotation);

                _compareOptions.compare = timeline::Compare::Wipe;
                _drawViewport(
                    feather_tk::Box2I(
                        0,
                        viewportSize.y + viewportSpacing,
                        viewportSize.x,
                        viewportSize.y),
                    fontSize,
                    _compareOptions,
                    _rotation);
                _compareOptions.compare = timeline::Compare::Overlay;
                _drawViewport(
                    feather_tk::Box2I(
                        viewportSize.x + viewportSpacing,
                        viewportSize.y + viewportSpacing,
                        viewportSize.x,
                        viewportSize.y),
                    fontSize,
                    _compareOptions,
                    _rotation);
                _compareOptions.compare = timeline::Compare::Difference;
                _drawViewport(
                    feather_tk::Box2I(
                        viewportSize.x * 2 + viewportSpacing * 2,
                        viewportSize.y + viewportSpacing,
                        viewportSize.x,
                        viewportSize.y),
                    fontSize,
                    _compareOptions,
                    _rotation);

                _compareOptions.compare = timeline::Compare::Horizontal;
                _drawViewport(
                    feather_tk::Box2I(
                        0,
                        viewportSize.y * 2 + viewportSpacing * 2,
                        viewportSize.x,
                        viewportSize.y),
                    fontSize,
                    _compareOptions,
                    _rotation);
                _compareOptions.compare = timeline::Compare::Vertical;
                _drawViewport(
                    feather_tk::Box2I(
                        viewportSize.x + viewportSpacing,
                        viewportSize.y * 2 + viewportSpacing * 2,
                        viewportSize.x,
                        viewportSize.y),
                    fontSize,
                    _compareOptions,
                    _rotation);
                _compareOptions.compare = timeline::Compare::Tile;
                _drawViewport(
                    feather_tk::Box2I(
                        viewportSize.x * 2 + viewportSpacing * 2,
                        viewportSize.y * 2 + viewportSpacing * 2,
                        viewportSize.x,
                        viewportSize.y),
                    fontSize,
                    _compareOptions,
                    _rotation);
            }

            void App::_drawViewport(
                const feather_tk::Box2I& box,
                uint16_t fontSize,
                const timeline::CompareOptions& compareOptions,
                float rotation)
            {
                const feather_tk::Size2I viewportSize = box.size();
                const float viewportAspect = feather_tk::aspectRatio(viewportSize);
                const feather_tk::Size2I renderSize = timeline::getRenderSize(
                    compareOptions.compare,
                    _videoData);
                const float renderSizeAspect = feather_tk::aspectRatio(renderSize);
                feather_tk::Size2I transformSize;
                feather_tk::V2F transformOffset;
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
                _render->clearViewport(feather_tk::Color4F(0.F, 0.F, 0.F));

                _render->setTransform(feather_tk::ortho(
                    0.F,
                    static_cast<float>(transformSize.w),
                    static_cast<float>(transformSize.h),
                    0.F,
                    -1.F,
                    1.F) *
                    feather_tk::translate(feather_tk::V3F(transformOffset.x, transformOffset.y, 0.F)) *
                    feather_tk::rotateZ(rotation) *
                    feather_tk::translate(feather_tk::V3F(-renderSize.w / 2, -renderSize.h / 2, 0.F)));
                _render->drawVideo(
                    _videoData,
                    timeline::getBoxes(compareOptions.compare, _videoData),
                    {},
                    {},
                    compareOptions);

                if (_hud)
                {
                    _render->setTransform(feather_tk::ortho(
                        0.F,
                        static_cast<float>(viewportSize.w),
                        static_cast<float>(viewportSize.h),
                        0.F,
                        -1.F,
                        1.F));

                    feather_tk::FontInfo fontInfo;
                    fontInfo.size = fontSize;
                    auto fontSystem = _context->getSystem<feather_tk::FontSystem>();
                    auto fontMetrics = fontSystem->getMetrics(fontInfo);
                    std::string text = timeline::getLabel(compareOptions.compare);
                    feather_tk::Size2I textSize = fontSystem->getSize(text, fontInfo);
                    _render->drawRect(
                        feather_tk::Box2I(0, 0, viewportSize.w, fontMetrics.lineHeight),
                        feather_tk::Color4F(0.F, 0.F, 0.F, .7F));
                    _render->drawText(
                        fontSystem->getGlyphs(text, fontInfo),
                        fontMetrics,
                        feather_tk::V2I(fontSize / 5, 0),
                        feather_tk::Color4F(1.F, 1.F, 1.F));
                }

                _render->setClipRectEnabled(false);
            }

            void App::_hudCallback(bool value)
            {
                _hud = value;
                _renderDirty = true;
                _context->getLogSystem()->print("render", feather_tk::Format("HUD: {0}").arg(_hud));
            }

            void App::_playbackCallback(timeline::Playback value)
            {
                _player->setPlayback(value);
                _context->getLogSystem()->print("render", feather_tk::Format("Playback: {0}").arg(_player->observePlayback()->get()));
            }
        }
    }
}
