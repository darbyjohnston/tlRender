// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2023 Darby Johnston
// All rights reserved.

#include <tlPlayGLApp/App.h>

#include <tlPlayGLApp/MainWindow.h>

#include <tlUI/EventLoop.h>
#include <tlUI/FileBrowser.h>

#include <tlTimeline/Util.h>

#include <tlIO/IOSystem.h>

#include <tlCore/AudioSystem.h>
#include <tlCore/File.h>
#include <tlCore/StringFormat.h>

#if defined(TLRENDER_NFD)
#include <nfd.hpp>
#endif // TLRENDER_NFD

namespace tl
{
    namespace play_gl
    {
        struct App::Private
        {
            std::string input;
            Options options;
            std::shared_ptr<play::FilesModel> filesModel;
            std::vector<std::shared_ptr<play::FilesModelItem> > active;
            float volume = 1.F;
            bool mute = false;
            bool deviceActive = false;
            std::shared_ptr<observer::Value<std::shared_ptr<timeline::Player> > > player;
            std::shared_ptr<observer::List<std::shared_ptr<timeline::Player> > > players;

            std::shared_ptr<MainWindow> mainWindow;
            std::shared_ptr<ui::FileBrowser> fileBrowser;

            std::shared_ptr<observer::ListObserver<std::shared_ptr<play::FilesModelItem> > > activeObserver;
            std::shared_ptr<observer::ListObserver<int> > layersObserver;
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
                "play-gl",
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
                app::CmdLineValueOption<double>::create(
                    p.options.speed,
                    { "-speed" },
                    "Playback speed."),
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
            const int exitCode = getExit();
            if (exitCode != 0)
            {
                exit(exitCode);
                return;
            }

            // Set I/O options.
            io::Options ioOptions;
#if defined(TLRENDER_USD)
            {
                std::stringstream ss;
                ss << p.options.usdRenderWidth;
                ioOptions["USD/renderWidth"] = ss.str();
            }
            {
                std::stringstream ss;
                ss << p.options.usdComplexity;
                ioOptions["USD/complexity"] = ss.str();
            }
            {
                std::stringstream ss;
                ss << p.options.usdDrawMode;
                ioOptions["USD/drawMode"] = ss.str();
            }
            {
                std::stringstream ss;
                ss << p.options.usdEnableLighting;
                ioOptions["USD/enableLighting"] = ss.str();
            }
            {
                std::stringstream ss;
                ss << p.options.usdStageCache;
                ioOptions["USD/stageCacheCount"] = ss.str();
            }
            {
                std::stringstream ss;
                ss << p.options.usdDiskCache * memory::gigabyte;
                ioOptions["USD/diskCacheByteCount"] = ss.str();
            }
#endif // TLRENDER_USD
            auto ioSystem = context->getSystem<io::System>();
            ioSystem->setOptions(ioOptions);

            // Create objects.
            p.filesModel = play::FilesModel::create(context);
            p.player = observer::Value<std::shared_ptr<timeline::Player> >::create();
            p.players = observer::List<std::shared_ptr<timeline::Player> >::create();
            p.activeObserver = observer::ListObserver<std::shared_ptr<play::FilesModelItem> >::create(
                p.filesModel->observeActive(),
                [this](const std::vector<std::shared_ptr<play::FilesModelItem> >& value)
                {
                    _activeCallback(value);
                });
            p.layersObserver = observer::ListObserver<int>::create(
                p.filesModel->observeLayers(),
                [this](const std::vector<int>& value)
                {
                    for (size_t i = 0; i < value.size() && i < _p->players->getSize(); ++i)
                    {
                        if (auto player = _p->players->getItem(i))
                        {
                            player->setVideoLayer(value[i]);
                        }
                    }
                });

            // Open the input files.
            if (!p.input.empty())
            {
                open(p.input);
                if (auto player = p.player->get())
                {
                    if (p.options.speed > 0.0)
                    {
                        player->setSpeed(p.options.speed);
                    }
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

        void App::openDialog()
        {
            TLRENDER_P();
#if defined(TLRENDER_NFD)
            nfdu8char_t* outPath = nullptr;
            NFD::OpenDialog(outPath);
            if (outPath)
            {
                open(outPath);
                NFD::FreePath(outPath);
            }
#else  // TLRENDER_NFD
            std::string path;
            if (auto player = p.player->get())
            {
                path = player->getPath().get();
            }
            else
            {
                path = file::getCWD();
            }
            p.fileBrowser = ui::FileBrowser::create(path, _context);
            p.fileBrowser->open(getEventLoop());
            p.fileBrowser->setFileCallback(
                [this](const std::string& value)
                {
                    open(value);
                    _p->fileBrowser->close();
                });
            p.fileBrowser->setCloseCallback(
                [this]
                {
                    _p->fileBrowser.reset();
                });
#endif // TLRENDER_NFD
        }

        void App::open(const std::string& fileName, const std::string& audioFileName)
        {
            TLRENDER_P();
            file::PathOptions pathOptions;
            //pathOptions.maxNumberDigits = p.settingsObject->value("Misc/MaxFileSequenceDigits").toInt();
            for (const auto& path : timeline::getPaths(fileName, pathOptions, _context))
            {
                auto item = std::make_shared<play::FilesModelItem>();
                item->path = path;
                item->audioPath = file::Path(audioFileName);
                p.filesModel->add(item);
                //p.settingsObject->addRecentFile(QString::fromUtf8(path.get().c_str()));
            }
        }

        const std::shared_ptr<play::FilesModel>& App::getFilesModel() const
        {
            return _p->filesModel;
        }

        std::shared_ptr<observer::IValue<std::shared_ptr<timeline::Player> > > App::observePlayer() const
        {
            return _p->player;
        }

        std::shared_ptr<observer::IList<std::shared_ptr<timeline::Player> > > App::observePlayers() const
        {
            return _p->players;
        }

        const std::shared_ptr<MainWindow>& App::getMainWindow() const
        {
            return _p->mainWindow;
        }

        void App::_drop(const std::vector<std::string>& value)
        {
            for (const auto& i : value)
            {
                open(i);
            }
        }

        void App::_tick()
        {
            TLRENDER_P();
            for (const auto& player : p.players->get())
            {
                player->tick();
            }
        }

        void App::_activeCallback(const std::vector<std::shared_ptr<play::FilesModelItem> >& items)
        {
            TLRENDER_P();

            // Save the previous timeline player state.
            auto players = p.players->get();
            if (!p.active.empty() &&
                !players.empty() &&
                players[0])
            {
                p.active[0]->speed = players[0]->getSpeed();
                p.active[0]->playback = players[0]->getPlayback();
                p.active[0]->loop = players[0]->getLoop();
                p.active[0]->currentTime = players[0]->getCurrentTime();
                p.active[0]->inOutRange = players[0]->getInOutRange();
                p.active[0]->videoLayer = players[0]->getVideoLayer();
                p.active[0]->audioOffset = players[0]->getAudioOffset();
            }

            // Find the timeline players that need to be created.
            std::vector<std::shared_ptr<timeline::Player> > playersNew(items.size());
            std::vector<std::shared_ptr<play::FilesModelItem> > active = p.active;
            for (size_t i = 0; i < items.size(); ++i)
            {
                const auto j = std::find(active.begin(), active.end(), items[i]);
                if (j != active.end())
                {
                    const size_t k = j - active.begin();
                    playersNew[i] = players[k];
                    active.erase(j);
                }
            }

            // Find the timeline players that need to be destroyed.
            std::vector<std::shared_ptr<timeline::Player> > playersToDestroy;
            for (size_t i = 0; i < p.active.size(); ++i)
            {
                const auto j = std::find(items.begin(), items.end(), p.active[i]);
                if (j == items.end())
                {
                    playersToDestroy.push_back(players[i]);
                }
                players[i] = nullptr;
            }

            // Create new timeline players.
            auto audioSystem = _context->getSystem<audio::System>();
            for (size_t i = 0; i < items.size(); ++i)
            {
                const auto& item = items[i];
                if (!playersNew[i])
                {
                    try
                    {
                        timeline::Options options;
                        /*options.fileSequenceAudio = p.settingsObject->value("FileSequence/Audio").
                            value<timeline::FileSequenceAudio>();
                        options.fileSequenceAudioFileName = p.settingsObject->value("FileSequence/AudioFileName").
                            toString().toUtf8().data();
                        options.fileSequenceAudioDirectory = p.settingsObject->value("FileSequence/AudioDirectory").
                            toString().toUtf8().data();
                        options.videoRequestCount = p.settingsObject->value("Performance/VideoRequestCount").toInt();
                        options.audioRequestCount = p.settingsObject->value("Performance/AudioRequestCount").toInt();
                        options.ioOptions["SequenceIO/ThreadCount"] = string::Format("{0}").
                            arg(p.settingsObject->value("Performance/SequenceThreadCount").toInt());
                        options.ioOptions["FFmpeg/YUVToRGBConversion"] = string::Format("{0}").
                            arg(p.settingsObject->value("Performance/FFmpegYUVToRGBConversion").toBool());
                        options.ioOptions["FFmpeg/ThreadCount"] = string::Format("{0}").
                            arg(p.settingsObject->value("Performance/FFmpegThreadCount").toInt());
                        options.pathOptions.maxNumberDigits = std::min(
                            p.settingsObject->value("Misc/MaxFileSequenceDigits").toInt(),
                            255);*/
                        auto otioTimeline = item->audioPath.isEmpty() ?
                            timeline::create(item->path.get(), _context, options) :
                            timeline::create(item->path.get(), item->audioPath.get(), _context, options);
                        auto timeline = timeline::Timeline::create(otioTimeline, _context, options);
                        const otime::TimeRange& timeRange = timeline->getTimeRange();

                        timeline::PlayerOptions playerOptions;
                        playerOptions.cache.readAhead = _cacheReadAhead();
                        playerOptions.cache.readBehind = _cacheReadBehind();
                        //playerOptions.timerMode = p.settingsObject->value("Performance/TimerMode").
                        //    value<timeline::TimerMode>();
                        //playerOptions.audioBufferFrameCount =
                        //    p.settingsObject->value("Performance/AudioBufferFrameCount").toInt();
                        if (item->init)
                        {
                            if (0 == i)
                            {
                                playerOptions.currentTime = items[0]->currentTime;
                            }
                            else
                            {
                                playerOptions.currentTime = timeline::getExternalTime(
                                    items[0]->currentTime,
                                    items[0]->timeRange,
                                    timeRange,
                                    playerOptions.externalTimeMode);
                            }
                        }
                        playersNew[i] = timeline::Player::create(timeline, _context, playerOptions);
                    }
                    catch (const std::exception& e)
                    {
                        _log(e.what(), log::Type::Error);
                    }
                }
            }

            // Initialize timeline players.
            for (size_t i = 0; i < items.size(); ++i)
            {
                const auto& item = items[i];
                auto& player = playersNew[i];
                if (player)
                {
                    item->timeRange = player->getTimeRange();
                    item->ioInfo = player->getIOInfo();

                    if (!item->init)
                    {
                        item->init = true;
                        item->speed = player->getSpeed();
                        item->playback = player->getPlayback();
                        item->loop = player->getLoop();
                        item->currentTime = player->getCurrentTime();
                        item->inOutRange = player->getInOutRange();
                        item->videoLayer = player->getVideoLayer();
                        item->audioOffset = player->getAudioOffset();
                    }
                    else if (0 == i)
                    {
                        player->setSpeed(items[0]->speed);
                        player->setLoop(items[0]->loop);
                        player->setInOutRange(items[0]->inOutRange);
                        player->setVideoLayer(items[0]->videoLayer);
                        player->setVolume(p.volume);
                        player->setMute(p.mute);
                        player->setAudioOffset(items[0]->audioOffset);
                        player->setPlayback(items[0]->playback);
                    }

                    if (0 == i)
                    {
                        player->setExternalTime(nullptr);
                    }
                    else
                    {
                        player->setVideoLayer(items[i]->videoLayer);
                        player->setExternalTime(playersNew[0]);
                    }
                }
            }

            p.active = items;
            p.player->setIfChanged(!playersNew.empty() ? playersNew[0] : nullptr);
            p.players->setIfChanged(playersNew);
            playersToDestroy.clear();

            _cacheUpdate();
            _audioUpdate();
        }

        otime::RationalTime App::_cacheReadAhead() const
        {
            TLRENDER_P();
            const double readAhead = 4.0; //p.settingsObject->value("Cache/ReadAhead").toDouble();
            const size_t activeCount = p.filesModel->observeActive()->getSize();
            return otime::RationalTime(
                readAhead / static_cast<double>(activeCount),
                1.0);
        }

        otime::RationalTime App::_cacheReadBehind() const
        {
            TLRENDER_P();
            const double readBehind = 0.25; //p.settingsObject->value("Cache/ReadBehind").toDouble();
            const size_t activeCount = p.filesModel->observeActive()->getSize();
            return otime::RationalTime(
                readBehind / static_cast<double>(activeCount),
                1.0);
        }

        void App::_cacheUpdate()
        {
            TLRENDER_P();
            timeline::PlayerCacheOptions options;
            options.readAhead = _cacheReadAhead();
            options.readBehind = _cacheReadBehind();
            for (const auto& i : p.players->get())
            {
                if (i)
                {
                    i->setCacheOptions(options);
                }
            }
        }

        void App::_audioUpdate()
        {
            TLRENDER_P();
            for (const auto& i : p.players->get())
            {
                if (i)
                {
                    i->setVolume(p.volume);
                    i->setMute(p.mute || p.deviceActive);
                }
            }
        }
    }
}
