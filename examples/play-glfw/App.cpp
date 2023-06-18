// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2023 Darby Johnston
// All rights reserved.

#include "App.h"

#include "MainWindow.h"

#include <tlUI/EventLoop.h>

#include <tlIO/IOSystem.h>

#include <tlCore/StringFormat.h>

namespace tl
{
    namespace examples
    {
        namespace play_glfw
        {
            struct App::Private
            {
                std::string input;
                Options options;
                std::shared_ptr<observer::Value<std::shared_ptr<timeline::Player> > > player;
                std::shared_ptr<observer::List<std::shared_ptr<timeline::Player> > > players;
                std::shared_ptr<MainWindow> mainWindow;
            };

            void App::_init(
                int argc,
                char* argv[],
                const std::shared_ptr<system::Context>& context)
            {
                TLRENDER_P();
                IApp::_init(
                    argc,
                    argv,
                    context,
                    "play-glfw",
                    "Example GLFW playback application.",
                    {
                        app::CmdLineValueArg<std::string>::create(
                            p.input,
                            "input",
                            "The input timeline, movie, or image sequence.",
                            true)
                    },
                {
                    app::CmdLineValueOption<std::string>::create(
                        p.options.compareFileName,
                        { "-compare", "-b" },
                        "A/B comparison \"B\" file name."),
                    app::CmdLineValueOption<timeline::CompareMode>::create(
                        p.options.compareOptions.mode,
                        { "-compareMode", "-c" },
                        "A/B comparison mode.",
                        string::Format("{0}").arg(p.options.compareOptions.mode),
                        string::join(timeline::getCompareModeLabels(), ", ")),
                    app::CmdLineValueOption<math::Vector2f>::create(
                        p.options.compareOptions.wipeCenter,
                        { "-wipeCenter", "-wc" },
                        "A/B comparison wipe center.",
                        string::Format("{0}").arg(p.options.compareOptions.wipeCenter)),
                    app::CmdLineValueOption<float>::create(
                        p.options.compareOptions.wipeRotation,
                        { "-wipeRotation", "-wr" },
                        "A/B comparison wipe rotation.",
                        string::Format("{0}").arg(p.options.compareOptions.wipeRotation)),
                    app::CmdLineValueOption<imaging::Size>::create(
                        p.options.windowSize,
                        { "-windowSize", "-ws" },
                        "Window size.",
                        string::Format("{0}x{1}").arg(p.options.windowSize.w).arg(p.options.windowSize.h)),
                    app::CmdLineFlagOption::create(
                        p.options.fullscreen,
                        { "-fullscreen", "-fs" },
                        "Enable full screen mode."),
                    app::CmdLineValueOption<bool>::create(
                        p.options.hud,
                        { "-hud" },
                        "Enable the HUD (heads up display).",
                        string::Format("{0}").arg(p.options.hud),
                        "0, 1"),
                    app::CmdLineValueOption<timeline::Playback>::create(
                        p.options.playback,
                        { "-playback", "-p" },
                        "Playback mode.",
                        string::Format("{0}").arg(p.options.playback),
                        string::join(timeline::getPlaybackLabels(), ", ")),
                    app::CmdLineValueOption<timeline::Loop>::create(
                        p.options.loop,
                        { "-loop", "-lp" },
                        "Playback loop mode.",
                        string::Format("{0}").arg(p.options.loop),
                        string::join(timeline::getLoopLabels(), ", ")),
                    app::CmdLineValueOption<otime::RationalTime>::create(
                        p.options.seek,
                        { "-seek" },
                        "Seek to the given time."),
                    app::CmdLineValueOption<otime::TimeRange>::create(
                        p.options.inOutRange,
                        { "-inOutRange" },
                        "Set the in/out points range."),
                    app::CmdLineValueOption<std::string>::create(
                        p.options.colorConfigOptions.fileName,
                        { "-colorConfig", "-cc" },
                        "Color configuration file name (e.g., config.ocio)."),
                    app::CmdLineValueOption<std::string>::create(
                        p.options.colorConfigOptions.input,
                        { "-colorInput", "-ci" },
                        "Input color space."),
                    app::CmdLineValueOption<std::string>::create(
                        p.options.colorConfigOptions.display,
                        { "-colorDisplay", "-cd" },
                        "Display color space."),
                    app::CmdLineValueOption<std::string>::create(
                        p.options.colorConfigOptions.view,
                        { "-colorView", "-cv" },
                        "View color space."),
                    app::CmdLineValueOption<std::string>::create(
                        p.options.lutOptions.fileName,
                        { "-lut" },
                        "LUT file name."),
                    app::CmdLineValueOption<timeline::LUTOrder>::create(
                        p.options.lutOptions.order,
                        { "-lutOrder" },
                        "LUT operation order.",
                        string::Format("{0}").arg(p.options.lutOptions.order),
                        string::join(timeline::getLUTOrderLabels(), ", ")),
#if defined(TLRENDER_USD)
                    app::CmdLineValueOption<size_t>::create(
                        p.options.usdRenderWidth,
                        { "-usdRenderWidth" },
                        "USD render width.",
                        string::Format("{0}").arg(p.options.usdRenderWidth)),
                    app::CmdLineValueOption<float>::create(
                        p.options.usdComplexity,
                        { "-usdComplexity" },
                        "USD render complexity setting.",
                        string::Format("{0}").arg(p.options.usdComplexity)),
                    app::CmdLineValueOption<usd::DrawMode>::create(
                        p.options.usdDrawMode,
                        { "-usdDrawMode" },
                        "USD render draw mode.",
                        string::Format("{0}").arg(p.options.usdDrawMode),
                        string::join(usd::getDrawModeLabels(), ", ")),
                    app::CmdLineValueOption<bool>::create(
                        p.options.usdEnableLighting,
                        { "-usdEnableLighting" },
                        "USD render enable lighting setting.",
                        string::Format("{0}").arg(p.options.usdEnableLighting)),
                    app::CmdLineValueOption<size_t>::create(
                        p.options.usdStageCache,
                        { "-usdStageCache" },
                        "USD stage cache size.",
                        string::Format("{0}").arg(p.options.usdStageCache)),
                    app::CmdLineValueOption<size_t>::create(
                        p.options.usdDiskCache,
                        { "-usdDiskCache" },
                        "USD disk cache size in gigabytes. A size of zero disables the disk cache.",
                        string::Format("{0}").arg(p.options.usdDiskCache)),
#endif // TLRENDER_USD
                });

                // Set I/O options.
                io::Options ioOptions;
#if defined(TLRENDER_USD)
                {
                    std::stringstream ss;
                    ss << p.options.usdRenderWidth;
                    ioOptions["usd/renderWidth"] = ss.str();
                }
                {
                    std::stringstream ss;
                    ss << p.options.usdComplexity;
                    ioOptions["usd/complexity"] = ss.str();
                }
                {
                    std::stringstream ss;
                    ss << p.options.usdDrawMode;
                    ioOptions["usd/drawMode"] = ss.str();
                }
                {
                    std::stringstream ss;
                    ss << p.options.usdEnableLighting;
                    ioOptions["usd/enableLighting"] = ss.str();
                }
                {
                    std::stringstream ss;
                    ss << p.options.usdStageCache;
                    ioOptions["usd/stageCacheCount"] = ss.str();
                }
                {
                    std::stringstream ss;
                    ss << p.options.usdDiskCache * memory::gigabyte;
                    ioOptions["usd/diskCacheByteCount"] = ss.str();
                }
#endif // TLRENDER_USD
                auto ioSystem = context->getSystem<io::System>();
                ioSystem->setOptions(ioOptions);

                // Read the timeline.
                p.player = observer::Value<std::shared_ptr<timeline::Player> >::create();
                p.players = observer::List<std::shared_ptr<timeline::Player> >::create();
                if (!p.input.empty())
                {
                    open(p.input);
                    if (auto player = p.player->get())
                    {
                        if (time::isValid(p.options.inOutRange))
                        {
                            player->setInOutRange(p.options.inOutRange);
                            player->seek(p.options.inOutRange.start_time());
                        }
                        if (time::isValid(p.options.seek))
                        {
                            player->seek(p.options.seek);
                        }
                        player->setLoop(p.options.loop);
                        player->setPlayback(p.options.playback);
                    }
                }

                // Create the main window.
                p.mainWindow = MainWindow::create(
                    std::dynamic_pointer_cast<App>(shared_from_this()),
                    _context);
                getEventLoop()->addWidget(p.mainWindow);
            }

            App::App() :
                _p(new Private)
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

            void App::open(const std::string& fileName)
            {
                TLRENDER_P();
                try
                {
                    auto timeline = timeline::Timeline::create(fileName, _context);
                    auto player = timeline::Player::create(timeline, _context);
                    p.player->setIfChanged(player);
                    p.players->setIfChanged({ player });
                }
                catch (const std::exception& e)
                {
                    std::cerr << e.what() << std::endl;
                    p.player->setIfChanged(nullptr);
                    p.players->clear();
                }
            }

            void App::close()
            {
                TLRENDER_P();
                p.player->setIfChanged(nullptr);
                p.players->clear();
            }

            void App::closeAll()
            {
                TLRENDER_P();
                p.player->setIfChanged(nullptr);
                p.players->clear();
            }

            std::shared_ptr<observer::IValue<std::shared_ptr<timeline::Player> > > App::observePlayer() const
            {
                return _p->player;
            }

            std::shared_ptr<observer::IList<std::shared_ptr<timeline::Player> > > App::observePlayers() const
            {
                return _p->players;
            }

            void App::_tick()
            {
                TLRENDER_P();
                for (const auto& player : p.players->get())
                {
                    player->tick();
                }
            }
        }
    }
}
