// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2023 Darby Johnston
// All rights reserved.

#include "App.h"

#include "MainWindow.h"

#include <tlUI/EventLoop.h>

#include <tlCore/StringFormat.h>

namespace tl
{
    namespace examples
    {
        namespace play_glfw
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
                    "play-glfw",
                    "Example GLFW playback application.",
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
                    app::CmdLineValueOption<timeline::CompareMode>::create(
                        _options.compareOptions.mode,
                        { "-compareMode", "-c" },
                        "A/B comparison mode.",
                        string::Format("{0}").arg(_options.compareOptions.mode),
                        string::join(timeline::getCompareModeLabels(), ", ")),
                    app::CmdLineValueOption<math::Vector2f>::create(
                        _options.compareOptions.wipeCenter,
                        { "-wipeCenter", "-wc" },
                        "A/B comparison wipe center.",
                        string::Format("{0}").arg(_options.compareOptions.wipeCenter)),
                    app::CmdLineValueOption<float>::create(
                        _options.compareOptions.wipeRotation,
                        { "-wipeRotation", "-wr" },
                        "A/B comparison wipe rotation.",
                        string::Format("{0}").arg(_options.compareOptions.wipeRotation)),
                    app::CmdLineValueOption<imaging::Size>::create(
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
                    app::CmdLineValueOption<timeline::Loop>::create(
                        _options.loop,
                        { "-loop", "-lp" },
                        "Playback loop mode.",
                        string::Format("{0}").arg(_options.loop),
                        string::join(timeline::getLoopLabels(), ", ")),
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

                // Read the timeline.
                auto timeline = timeline::Timeline::create(_input, _context);
                _player = timeline::Player::create(timeline, _context);

                // Create the main window.
                _mainWindow = MainWindow::create(_player, _context);
                getEventLoop()->addWidget(_mainWindow);

                // Initialize the timeline player.
                if (time::isValid(_options.inOutRange))
                {
                    _player->setInOutRange(_options.inOutRange);
                    _player->seek(_options.inOutRange.start_time());
                }
                if (time::isValid(_options.seek))
                {
                    _player->seek(_options.seek);
                }
                _player->setLoop(_options.loop);
                //_player->setPlayback(_options.playback);
            }

            App::App()
            {}

            App::~App()
            {}

            std::shared_ptr<App> App::create(
                int argc,
                char* argv[],
                const std::shared_ptr<system::Context>& context)
            {
                auto out = std::shared_ptr<App>(new App);
                out->_init(argc, argv, context);
                return out;
            }

            void App::_tick()
            {
                _player->tick();
            }
        }
    }
}
