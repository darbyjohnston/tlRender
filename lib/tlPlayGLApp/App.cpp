// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2023 Darby Johnston
// All rights reserved.

#include <tlPlayGLApp/App.h>

#include <tlPlayGLApp/MainWindow.h>
#include <tlPlayGLApp/SeparateAudioDialog.h>
#include <tlPlayGLApp/Settings.h>
#include <tlPlayGLApp/Tools.h>

#include <tlUI/EventLoop.h>
#include <tlUI/FileBrowser.h>

#include <tlTimeline/Util.h>

#include <tlIO/IOSystem.h>

#include <tlCore/AudioSystem.h>
#include <tlCore/File.h>
#include <tlCore/StringFormat.h>

namespace tl
{
    namespace play_gl
    {
        namespace
        {
            struct Options
            {
                std::string fileName;
                std::string audioFileName;
                std::string compareFileName;
                timeline::CompareOptions compareOptions;
                imaging::Size windowSize = imaging::Size(1280, 720);
                bool fullscreen = false;
                bool hud = true;
                double speed = 0.0;
                timeline::Playback playback = timeline::Playback::Stop;
                timeline::Loop loop = timeline::Loop::Loop;
                otime::RationalTime seek = time::invalidTime;
                otime::TimeRange inOutRange = time::invalidTimeRange;
                timeline::ColorConfigOptions colorConfigOptions;
                timeline::LUTOptions lutOptions;
#if defined(TLRENDER_USD)
                size_t usdRenderWidth = usd::RenderOptions().renderWidth;
                float usdComplexity = usd::RenderOptions().complexity;
                usd::DrawMode usdDrawMode = usd::RenderOptions().drawMode;
                bool usdEnableLighting = usd::RenderOptions().enableLighting;
                size_t usdStageCache = usd::RenderOptions().stageCacheCount;
                size_t usdDiskCache = usd::RenderOptions().diskCacheByteCount / memory::gigabyte;
#endif // TLRENDER_USD
                bool resetSettings = false;
                std::string settingsFileName;
            };
        }

        struct App::Private
        {
            std::string settingsFileName;
            std::shared_ptr<Settings> settings;
            Options options;
            float volume = 1.F;
            bool mute = false;
            bool deviceActive = false;
            timeline::PlayerCacheOptions playerCacheOptions;
            std::shared_ptr<play::FilesModel> filesModel;
            std::vector<std::shared_ptr<play::FilesModelItem> > files;
            std::vector<std::shared_ptr<play::FilesModelItem> > activeFiles;
            std::shared_ptr<ui::RecentFilesModel> recentFilesModel;
            std::shared_ptr<ToolsModel> toolsModel;
            std::vector<std::shared_ptr<timeline::Player> > players;
            std::shared_ptr<observer::List<std::shared_ptr<timeline::Player> > > activePlayers;

            std::shared_ptr<MainWindow> mainWindow;
            std::shared_ptr<SeparateAudioDialog> separateAudioDialog;

            std::shared_ptr<observer::ListObserver<std::shared_ptr<play::FilesModelItem> > > filesObserver;
            std::shared_ptr<observer::ListObserver<std::shared_ptr<play::FilesModelItem> > > activeObserver;
            std::shared_ptr<observer::ListObserver<int> > layersObserver;
            std::shared_ptr<observer::ValueObserver<std::string> > settingsObserver;
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
                        p.options.fileName,
                        "input",
                        "The input timeline, movie, or image sequence.",
                        true)
                },
            {
                app::CmdLineValueOption<std::string>::create(
                    p.options.audioFileName,
                    { "-audio", "-a" },
                    "Audio file name."),
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
                app::CmdLineFlagOption::create(
                    p.options.resetSettings,
                    { "-resetSettings" },
                    "Reset settings to defaults."),
                app::CmdLineValueOption<std::string>::create(
                    p.options.settingsFileName,
                    { "-settings" },
                    "Settings file name."),
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

            // Initialize the settings.
            p.settings = Settings::create(context);
            p.settings->setValue("Files/RecentMax", 10);
            p.settings->setValue("Timeline/FrameView", true);
            p.settings->setValue("Timeline/StopOnScrub", false);
            p.settings->setValue("Timeline/Thumbnails", true);
            p.settings->setValue("Timeline/ThumbnailsSize", 100);
            p.settings->setValue("Timeline/Transitions", false);
            p.settings->setValue("Timeline/Markers", false);
            p.settings->setValue("Audio/Volume", p.volume);
            p.settings->setValue("Audio/Mute", p.mute);
            p.settings->setValue("Cache/ReadAhead",
                p.playerCacheOptions.readAhead.value());
            p.settings->setValue("Cache/ReadBehind",
                p.playerCacheOptions.readBehind.value());
            p.settings->setValue("FileSequence/Audio",
                timeline::FileSequenceAudio::BaseName);
            p.settings->setValue("FileSequence/AudioFileName", std::string());
            p.settings->setValue("FileSequence/AudioDirectory", std::string());
            p.settings->setValue("FileSequence/MaxDigits", 9);
            p.settings->setValue("FileBrowser/NativeFileDialog", true);
            p.settings->setValue("FileBrowser/Path", file::getCWD());
            p.settings->setValue("FileBrowser/Options", ui::FileBrowserOptions());
            p.settings->setValue("Performance/TimerMode",
                timeline::PlayerOptions().timerMode);
            p.settings->setValue("Performance/AudioBufferFrameCount",
                timeline::PlayerOptions().audioBufferFrameCount);
            p.settings->setValue("Performance/VideoRequestCount", 16);
            p.settings->setValue("Performance/AudioRequestCount", 16);
            p.settings->setValue("Performance/SequenceThreadCount", 16);
            p.settings->setValue("Performance/FFmpegYUVToRGBConversion", false);
            p.settings->setValue("Performance/FFmpegThreadCount", 0);
            p.settings->setValue("Misc/ToolTipsEnabled", true);
            if (!p.options.settingsFileName.empty())
            {
                p.settingsFileName = p.options.settingsFileName;
            }
            else
            {
                const std::string documentsPath =
                    file::getUserPath(file::UserPath::Documents);
                if (!file::exists(documentsPath))
                {
                    file::mkdir(documentsPath);
                }
                const std::string settingsPath =
                    file::Path(documentsPath, "tlplay-gl").get();
                if (!file::exists(settingsPath))
                {
                    file::mkdir(settingsPath);
                }
                p.settingsFileName = file::Path(settingsPath, "Settings.json").get();
            }
            if (!p.options.resetSettings)
            {
                p.settings->read(p.settingsFileName);
            }

            // Initialize the models.
            p.filesModel = play::FilesModel::create(context);
            p.recentFilesModel = ui::RecentFilesModel::create(context);
            int recentFilesMax = 0;
            p.settings->getValue("Files/RecentMax", recentFilesMax);
            p.recentFilesModel->setRecentMax(recentFilesMax);
            std::vector<std::string> recentFiles;
            p.settings->getValue("Files/Recent", recentFiles);
            for (const auto& recentFile : recentFiles)
            {
                p.recentFilesModel->addRecent(file::Path(recentFile));
            }
            p.toolsModel = ToolsModel::create();

            // Initialize the file browser.
            if (auto fileBrowserSystem = context->getSystem<ui::FileBrowserSystem>())
            {
                std::string path;
                p.settings->getValue("FileBrowser/Path", path);
                fileBrowserSystem->setPath(path);
                ui::FileBrowserOptions options;
                p.settings->getValue("FileBrowser/Options", options);
                fileBrowserSystem->setOptions(options);
            }

            // Create observers.
            p.activePlayers = observer::List<std::shared_ptr<timeline::Player> >::create();
            p.filesObserver = observer::ListObserver<std::shared_ptr<play::FilesModelItem> >::create(
                p.filesModel->observeFiles(),
                [this](const std::vector<std::shared_ptr<play::FilesModelItem> >& value)
                {
                    _filesCallback(value);
                });
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
                    for (size_t i = 0; i < value.size() && i < _p->players.size(); ++i)
                    {
                        if (auto player = _p->players[i])
                        {
                            player->setVideoLayer(value[i]);
                        }
                    }
                });
            p.settingsObserver = observer::ValueObserver<std::string>::create(
                p.settings->observeValues(),
                [this](const std::string&)
                {
                    TLRENDER_P();
                    {
                        float volume = 1.F;
                        p.settings->getValue("Audio/Volume", volume);
                        if (volume != p.volume)
                        {
                            p.volume = volume;
                            _audioUpdate();
                        }
                    }
                    {
                        bool mute = false;
                        p.settings->getValue("Audio/Mute", mute);
                        if (mute != p.mute)
                        {
                            p.mute = mute;
                            _audioUpdate();
                        }
                    }
                    {
                        double value = 0.0;
                        p.settings->getValue("Cache/ReadAhead", value);
                        if (value != p.playerCacheOptions.readAhead.value())
                        {
                            p.playerCacheOptions.readAhead = otime::RationalTime(value, 1.0);
                            _cacheUpdate();
                        }
                    }
                    {
                        double value = 0.0;
                        p.settings->getValue("Cache/ReadBehind", value);
                        if (value != p.playerCacheOptions.readBehind.value())
                        {
                            p.playerCacheOptions.readBehind = otime::RationalTime(value, 1.0);
                            _cacheUpdate();
                        }
                    }
                    if (auto fileBrowserSystem = _context->getSystem<ui::FileBrowserSystem>())
                    {
                        bool value = false;
                        p.settings->getValue("FileBrowser/NativeFileDialog", value);
                        fileBrowserSystem->setNativeFileDialog(value);
                    }
                });

            // Open the input files.
            if (!p.options.fileName.empty())
            {
                if (!p.options.compareFileName.empty())
                {
                    open(p.options.compareFileName);
                    p.filesModel->setCompareOptions(p.options.compareOptions);
                    p.filesModel->setB(0, true);
                }

                open(p.options.fileName, p.options.audioFileName);
                
                if (!p.players.empty())
                {
                    if (auto player = p.players[0])
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
        {
            TLRENDER_P();

            // Save the settings.
            if (auto fileBrowserSystem = _context->getSystem<ui::FileBrowserSystem>())
            {
                p.settings->setValue(
                    "FileBrowser/Path",
                    fileBrowserSystem->getPath());
                p.settings->setValue(
                    "FileBrowser/Options",
                    fileBrowserSystem->getOptions());
            }
            std::vector<std::string> recentFiles;
            for (const auto& recentFile : p.recentFilesModel->getRecent())
            {
                recentFiles.push_back(recentFile.get());
            }
            p.settings->setValue("Files/Recent", recentFiles);
            p.settings->write(p.settingsFileName);
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

        void App::openDialog()
        {
            TLRENDER_P();
            if (auto fileBrowserSystem = _context->getSystem<ui::FileBrowserSystem>())
            {
                fileBrowserSystem->open(
                    getEventLoop(),
                    [this](const file::Path& value)
                    {
                        open(value.get());
                    });
            }
        }

        void App::openSeparateAudioDialog()
        {
            TLRENDER_P();
            p.separateAudioDialog = SeparateAudioDialog::create(_context);
            p.separateAudioDialog->open(getEventLoop());
            p.separateAudioDialog->setFileCallback(
                [this](const file::Path& value, const file::Path& audio)
                {
                    open(value.get(), audio.get());
                    _p->separateAudioDialog->close();
                });
            p.separateAudioDialog->setCloseCallback(
                [this]
                {
                    _p->separateAudioDialog.reset();
                });
        }

        void App::open(const std::string& fileName, const std::string& audioFileName)
        {
            TLRENDER_P();
            file::PathOptions pathOptions;
            p.settings->getValue("FileSequence/MaxDigits", pathOptions.maxNumberDigits);
            for (const auto& path : timeline::getPaths(fileName, pathOptions, _context))
            {
                auto item = std::make_shared<play::FilesModelItem>();
                item->path = path;
                item->audioPath = file::Path(audioFileName);
                p.filesModel->add(item);
                p.recentFilesModel->addRecent(item->path);
            }
        }

        const std::shared_ptr<Settings>& App::getSettings() const
        {
            return _p->settings;
        }

        const std::shared_ptr<play::FilesModel>& App::getFilesModel() const
        {
            return _p->filesModel;
        }

        const std::shared_ptr<ui::RecentFilesModel>& App::getRecentFilesModel() const
        {
            return _p->recentFilesModel;
        }

        std::shared_ptr<observer::IList<std::shared_ptr<timeline::Player> > > App::observeActivePlayers() const
        {
            return _p->activePlayers;
        }

        const std::shared_ptr<ToolsModel>& App::getToolsModel() const
        {
            return _p->toolsModel;
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
            for (const auto& player : p.players)
            {
                if (player)
                {
                    player->tick();
                }
            }
        }

        void App::_filesCallback(const std::vector<std::shared_ptr<play::FilesModelItem> >& items)
        {
            TLRENDER_P();

            // Create the new list of players.
            std::vector<std::shared_ptr<timeline::Player> > players(items.size());
            for (size_t i = 0; i < items.size(); ++i)
            {
                const auto j = std::find(p.files.begin(), p.files.end(), items[i]);
                if (j != p.files.end())
                {
                    const size_t k = j - p.files.begin();
                    players[i] = p.players[k];
                }
            }

            // Find players to destroy.
            std::vector<std::shared_ptr<timeline::Player> > destroy;
            for (size_t i = 0; i < p.files.size(); ++i)
            {
                const auto j = std::find(items.begin(), items.end(), p.files[i]);
                if (j == items.end())
                {
                    destroy.push_back(p.players[i]);
                }
            }

            // Create new timeline players.
            auto audioSystem = _context->getSystem<audio::System>();
            for (size_t i = 0; i < players.size(); ++i)
            {
                if (!players[i])
                {
                    try
                    {
                        timeline::Options options;
                        p.settings->getValue(
                            "FileSequence/Audio",
                            options.fileSequenceAudio);
                        p.settings->getValue(
                            "FileSequence/AudioFileName",
                            options.fileSequenceAudioFileName);
                        p.settings->getValue(
                            "FileSequence/AudioDirectory",
                            options.fileSequenceAudioDirectory);
                        p.settings->getValue(
                            "Performance/VideoRequestCount",
                            options.videoRequestCount);
                        p.settings->getValue(
                            "Performance/AudioRequestCount",
                            options.audioRequestCount);
                        int sequenceThreadCount = 0;
                        p.settings->getValue(
                            "Performance/SequenceThreadCount",
                            sequenceThreadCount);
                        options.ioOptions["SequenceIO/ThreadCount"] =
                            string::Format("{0}").arg(sequenceThreadCount);
                        bool ffmpegYUVToRGBConversion = false;
                        p.settings->getValue(
                            "Performance/FFmpegYUVToRGBConversion",
                            ffmpegYUVToRGBConversion);
                        options.ioOptions["FFmpeg/YUVToRGBConversion"] =
                            string::Format("{0}").arg(ffmpegYUVToRGBConversion);
                        int ffmpegThreadCount = 0;
                        p.settings->getValue(
                            "Performance/FFmpegThreadCount",
                            ffmpegThreadCount);
                        options.ioOptions["FFmpeg/ThreadCount"] =
                            string::Format("{0}").arg(ffmpegThreadCount);
                        p.settings->getValue(
                            "FileSequence/MaxDigits",
                            options.pathOptions.maxNumberDigits);
                        auto otioTimeline = items[i]->audioPath.isEmpty() ?
                            timeline::create(items[i]->path.get(), _context, options) :
                            timeline::create(items[i]->path.get(), items[i]->audioPath.get(), _context, options);
                        auto timeline = timeline::Timeline::create(otioTimeline, _context, options);

                        timeline::PlayerOptions playerOptions;
                        playerOptions.cache.readAhead = time::invalidTime;
                        playerOptions.cache.readBehind = time::invalidTime;
                        p.settings->getValue(
                            "Performance/TimerMode",
                            playerOptions.timerMode);
                        p.settings->getValue(
                            "Performance/AudioBufferFrameCount",
                            playerOptions.audioBufferFrameCount);
                        players[i] = timeline::Player::create(
                            timeline,
                            _context,
                            playerOptions);

                        for (const auto& video : players[i]->getIOInfo().video)
                        {
                            items[i]->videoLayers.push_back(video.name);
                        }
                    }
                    catch (const std::exception& e)
                    {
                        _log(e.what(), log::Type::Error);
                    }
                }
            }

            p.files = items;
            p.players = players;
        }

        void App::_activeCallback(const std::vector<std::shared_ptr<play::FilesModelItem> >& items)
        {
            TLRENDER_P();

            auto activePlayers = _getActivePlayers();
            if (!activePlayers.empty() && activePlayers[0])
            {
                activePlayers[0]->setPlayback(timeline::Playback::Stop);
            }

            p.activeFiles = items;

            activePlayers = _getActivePlayers();
            p.activePlayers->setIfChanged(activePlayers);
            std::shared_ptr<timeline::Player> first;
            if (!activePlayers.empty())
            {
                first = activePlayers[0];
                if (first)
                {
                    first->setExternalTime(nullptr);
                }
            }
            for (size_t i = 1; i < activePlayers.size(); ++i)
            {
                if (auto player = activePlayers[i])
                {
                    activePlayers[i]->setExternalTime(
                        first.get() != player.get() ?
                        first :
                        nullptr);
                }
            }

            _cacheUpdate();
            _audioUpdate();
        }

        std::vector<std::shared_ptr<timeline::Player> > App::_getActivePlayers() const
        {
            TLRENDER_P();
            std::vector<std::shared_ptr<timeline::Player> > out;
            for (size_t i = 0; i < p.activeFiles.size(); ++i)
            {
                const auto j = std::find(
                    p.files.begin(),
                    p.files.end(),
                    p.activeFiles[i]);
                if (j != p.files.end())
                {
                    const auto k = j - p.files.begin();
                    out.push_back(p.players[k]);
                }
            }
            return out;
        }

        otime::RationalTime App::_getCacheReadAhead() const
        {
            TLRENDER_P();
            const size_t activeCount = p.activeFiles.size();
            return otime::RationalTime(
                p.playerCacheOptions.readAhead.value() / static_cast<double>(activeCount),
                1.0);
        }

        otime::RationalTime App::_getCacheReadBehind() const
        {
            TLRENDER_P();
            const size_t activeCount = p.activeFiles.size();
            return otime::RationalTime(
                p.playerCacheOptions.readBehind.value() / static_cast<double>(activeCount),
                1.0);
        }

        void App::_cacheUpdate()
        {
            TLRENDER_P();

            const auto activePlayers = _getActivePlayers();

            // Update inactive players.
            timeline::PlayerCacheOptions cacheOptions;
            cacheOptions.readAhead = time::invalidTime;
            cacheOptions.readBehind = time::invalidTime;
            for (const auto& player : p.players)
            {
                const auto j = std::find(
                    activePlayers.begin(),
                    activePlayers.end(),
                    player);
                if (j == activePlayers.end())
                {
                    if (player)
                    {
                        player->setCacheOptions(cacheOptions);
                    }
                }
            }

            // Update active players.
            cacheOptions.readAhead = _getCacheReadAhead();
            cacheOptions.readBehind = _getCacheReadBehind();
            for (const auto& player : activePlayers)
            {
                if (player)
                {
                    player->setCacheOptions(cacheOptions);
                }
            }
        }

        void App::_audioUpdate()
        {
            TLRENDER_P();
            for (const auto& player : p.players)
            {
                if (player)
                {
                    player->setVolume(p.volume);
                    player->setMute(p.mute || p.deviceActive);
                }
            }
        }
    }
}
