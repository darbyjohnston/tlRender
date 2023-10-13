// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2023 Darby Johnston
// All rights reserved.

#include <tlPlayGLApp/App.h>

#include <tlPlayGLApp/MainWindow.h>
#include <tlPlayGLApp/SeparateAudioDialog.h>
#include <tlPlayGLApp/Style.h>
#include <tlPlayGLApp/Tools.h>

#include <tlPlay/AudioModel.h>
#include <tlPlay/ColorModel.h>
#include <tlPlay/FilesModel.h>
#include <tlPlay/Settings.h>
#include <tlPlay/ViewportModel.h>
#include <tlPlay/Util.h>

#include <tlUI/EventLoop.h>
#include <tlUI/FileBrowser.h>
#include <tlUI/RecentFilesModel.h>

#include <tlTimeline/Util.h>

#include <tlIO/System.h>

#include <tlCore/AudioSystem.h>
#include <tlCore/File.h>
#include <tlCore/FileLogSystem.h>
#include <tlCore/StringFormat.h>

#if defined(TLRENDER_USD)
#include <tlIO/USD.h>
#endif // TLRENDER_USD

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
                bool hud = true;
                double speed = 0.0;
                timeline::Playback playback = timeline::Playback::Stop;
                timeline::Loop loop = timeline::Loop::Loop;
                otime::RationalTime seek = time::invalidTime;
                otime::TimeRange inOutRange = time::invalidTimeRange;
                timeline::ColorConfigOptions colorConfigOptions;
                timeline::LUTOptions lutOptions;

#if defined(TLRENDER_USD)
                int usdRenderWidth = 1920;
                float usdComplexity = 1.F;
                usd::DrawMode usdDrawMode = usd::DrawMode::ShadedSmooth;
                bool usdEnableLighting = true;
                bool usdSRGB = true;
                size_t usdStageCache = 10;
                size_t usdDiskCache = 0;
#endif // TLRENDER_USD

                std::string logFileName;
                bool resetSettings = false;
                std::string settingsFileName;
            };
        }

        struct App::Private
        {
            Options options;
            std::shared_ptr<file::FileLogSystem> fileLogSystem;
            std::string settingsFileName;
            std::shared_ptr<play::Settings> settings;
            io::Options ioOptions;
            size_t cacheSize = 4;
            timeline::PlayerCacheOptions playerCacheOptions;
            std::shared_ptr<play::FilesModel> filesModel;
            std::vector<std::shared_ptr<play::FilesModelItem> > files;
            std::vector<std::shared_ptr<play::FilesModelItem> > activeFiles;
            std::vector<std::shared_ptr<timeline::Player> > players;
            std::shared_ptr<observer::List<std::shared_ptr<timeline::Player> > > activePlayers;
            std::shared_ptr<play::ViewportModel> viewportModel;
            std::shared_ptr<play::ColorModel> colorModel;
            bool deviceActive = false;
            std::shared_ptr<play::AudioModel> audioModel;
            std::shared_ptr<ToolsModel> toolsModel;

            std::shared_ptr<MainWindow> mainWindow;
            std::shared_ptr<SeparateAudioDialog> separateAudioDialog;

            std::shared_ptr<observer::ListObserver<std::shared_ptr<play::FilesModelItem> > > filesObserver;
            std::shared_ptr<observer::ListObserver<std::shared_ptr<play::FilesModelItem> > > activeObserver;
            std::shared_ptr<observer::ListObserver<int> > layersObserver;
            std::shared_ptr<observer::ValueObserver<timeline::ColorConfigOptions> > colorConfigOptionsObserver;
            std::shared_ptr<observer::ValueObserver<timeline::LUTOptions> > lutOptionsObserver;
            std::shared_ptr<observer::ValueObserver<float> > volumeObserver;
            std::shared_ptr<observer::ValueObserver<bool> > muteObserver;
            std::shared_ptr<observer::ValueObserver<double> > syncOffsetObserver;
            std::shared_ptr<observer::ValueObserver<std::string> > settingsObserver;
        };

        void App::_init(
            const std::vector<std::string>& argv,
            const std::shared_ptr<system::Context>& context)
        {
            TLRENDER_P();
            const std::string appName = "tlplay-gl";
            const std::string appDocsPath = play::appDocsPath();
            std::string logFileName = play::logFileName(appName, appDocsPath);
            const std::string settingsFileName =
                play::settingsName(appName, appDocsPath);
            IApp::_init(
                argv,
                context,
                appName,
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
                    { "-b" },
                    "A/B comparison \"B\" file name."),
                app::CmdLineValueOption<timeline::CompareMode>::create(
                    p.options.compareOptions.mode,
                    { "-compare", "-c" },
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
                app::CmdLineValueOption<int>::create(
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
                    "USD draw mode.",
                    string::Format("{0}").arg(p.options.usdDrawMode),
                    string::join(usd::getDrawModeLabels(), ", ")),
                app::CmdLineValueOption<bool>::create(
                    p.options.usdEnableLighting,
                    { "-usdEnableLighting" },
                    "USD enable lighting.",
                    string::Format("{0}").arg(p.options.usdEnableLighting)),
                app::CmdLineValueOption<bool>::create(
                    p.options.usdSRGB,
                    { "-usdSRGB" },
                    "USD enable sRGB color space.",
                    string::Format("{0}").arg(p.options.usdSRGB)),
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
                app::CmdLineValueOption<std::string>::create(
                    p.options.logFileName,
                    { "-logFile" },
                    "Log file name.",
                    string::Format("{0}").arg(logFileName)),
                app::CmdLineFlagOption::create(
                    p.options.resetSettings,
                    { "-resetSettings" },
                    "Reset settings to defaults."),
                app::CmdLineValueOption<std::string>::create(
                    p.options.settingsFileName,
                    { "-settings" },
                    "Settings file name.",
                    string::Format("{0}").arg(settingsFileName)),
            });
            const int exitCode = getExit();
            if (exitCode != 0)
            {
                exit(exitCode);
                return;
            }
            _fileLogInit(logFileName);
            _settingsInit(settingsFileName);
            _modelsInit();
            _observersInit();
            _inputFilesInit();
            _mainWindowInit();
        }

        App::App() :
            _p(new Private)
        {}

        App::~App()
        {
            TLRENDER_P();
            p.mainWindow.reset();
            if (p.settings)
            {
                p.settings->write(p.settingsFileName);
            }
        }

        std::shared_ptr<App> App::create(
            const std::vector<std::string>& argv,
            const std::shared_ptr<system::Context>& context)
        {
            auto out = std::shared_ptr<App>(new App);
            out->_init(argv, context);
            return out;
        }

        void App::openDialog()
        {
            TLRENDER_P();
            auto fileBrowserSystem = _context->getSystem<ui::FileBrowserSystem>();
            fileBrowserSystem->open(
                getEventLoop(),
                [this](const file::FileInfo& value)
                {
                    open(value.getPath());
                });
        }

        void App::openSeparateAudioDialog()
        {
            TLRENDER_P();
            p.separateAudioDialog = SeparateAudioDialog::create(_context);
            p.separateAudioDialog->open(getEventLoop());
            p.separateAudioDialog->setCallback(
                [this](const file::Path& value, const file::Path& audio)
                {
                    open(value, audio);
                    _p->separateAudioDialog->close();
                });
            p.separateAudioDialog->setCloseCallback(
                [this]
                {
                    _p->separateAudioDialog.reset();
                });
        }

        void App::open(const file::Path& path, const file::Path& audioPath)
        {
            TLRENDER_P();
            file::PathOptions pathOptions;
            p.settings->getValue("FileSequence/MaxDigits", pathOptions.maxNumberDigits);
            for (const auto& i : timeline::getPaths(path, pathOptions, _context))
            {
                auto item = std::make_shared<play::FilesModelItem>();
                item->path = i;
                item->audioPath = audioPath;
                p.filesModel->add(item);

                auto fileBrowserSystem = _context->getSystem<ui::FileBrowserSystem>();
                auto recentFilesModel = fileBrowserSystem->getRecentFilesModel();
                recentFilesModel->addRecent(path);
            }
        }

        const std::shared_ptr<play::Settings>& App::getSettings() const
        {
            return _p->settings;
        }

        const std::shared_ptr<play::FilesModel>& App::getFilesModel() const
        {
            return _p->filesModel;
        }

        std::shared_ptr<observer::IList<std::shared_ptr<timeline::Player> > > App::observeActivePlayers() const
        {
            return _p->activePlayers;
        }

        const std::shared_ptr<play::ViewportModel>& App::getViewportModel() const
        {
            return _p->viewportModel;
        }

        const std::shared_ptr<play::ColorModel>& App::getColorModel() const
        {
            return _p->colorModel;
        }

        const std::shared_ptr<play::AudioModel>& App::getAudioModel() const
        {
            return _p->audioModel;
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
                open(file::Path(i));
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

        void App::_fileLogInit(const std::string& logFileName)
        {
            TLRENDER_P();
            std::string logFileName2 = logFileName;
            if (!p.options.logFileName.empty())
            {
                logFileName2 = p.options.logFileName;
            }
            p.fileLogSystem = file::FileLogSystem::create(logFileName2, _context);
        }

        void App::_settingsInit(const std::string& settingsFileName)
        {
            TLRENDER_P();
            p.settings = play::Settings::create(_context);
            if (!p.options.settingsFileName.empty())
            {
                p.settingsFileName = p.options.settingsFileName;
            }
            else
            {
                p.settingsFileName = settingsFileName;
            }
            if (!p.options.resetSettings)
            {
                p.settings->read(p.settingsFileName);
            }
            p.settings->setDefaultValue("Files/RecentMax", 10);
            p.settings->setDefaultValue("Window/Size", _options.windowSize);
            p.settings->setDefaultValue("Viewport/Background",
                timeline::BackgroundOptions());
            p.settings->setDefaultValue("Audio/Volume", 1.F);
            p.settings->setDefaultValue("Audio/Mute", false);
            p.settings->setDefaultValue("Cache/Size", p.cacheSize);
            p.settings->setDefaultValue("Cache/ReadAhead",
                p.playerCacheOptions.readAhead.value());
            p.settings->setDefaultValue("Cache/ReadBehind",
                p.playerCacheOptions.readBehind.value());
            p.settings->setDefaultValue("FileSequence/Audio",
                timeline::FileSequenceAudio::BaseName);
            p.settings->setDefaultValue("FileSequence/AudioFileName", std::string());
            p.settings->setDefaultValue("FileSequence/AudioDirectory", std::string());
            p.settings->setDefaultValue("FileSequence/MaxDigits", 9);
            p.settings->setDefaultValue("SequenceIO/ThreadCount", 16);
#if defined(TLRENDER_FFMPEG)
            p.settings->setDefaultValue("FFmpeg/YUVToRGBConversion", false);
            p.settings->setDefaultValue("FFmpeg/ThreadCount", 0);
#endif // TLRENDER_FFMPEG
#if defined(TLRENDER_USD)
            p.settings->setDefaultValue("USD/renderWidth", p.options.usdRenderWidth);
            p.settings->setDefaultValue("USD/complexity", p.options.usdComplexity);
            p.settings->setDefaultValue("USD/drawMode", p.options.usdDrawMode);
            p.settings->setDefaultValue("USD/enableLighting", p.options.usdEnableLighting);
            p.settings->setDefaultValue("USD/sRGB", p.options.usdSRGB);
            p.settings->setDefaultValue("USD/stageCacheCount", p.options.usdStageCache);
            p.settings->setDefaultValue("USD/diskCacheByteCount", p.options.usdDiskCache);
#endif // TLRENDER_USD
            p.settings->setDefaultValue("FileBrowser/NativeFileDialog", true);
            p.settings->setDefaultValue("FileBrowser/Path", file::getCWD());
            p.settings->setDefaultValue("FileBrowser/Options", ui::FileBrowserOptions());
            p.settings->setDefaultValue("Performance/TimerMode",
                timeline::PlayerOptions().timerMode);
            p.settings->setDefaultValue("Performance/AudioBufferFrameCount",
                timeline::PlayerOptions().audioBufferFrameCount);
            p.settings->setDefaultValue("Performance/VideoRequestCount", 16);
            p.settings->setDefaultValue("Performance/AudioRequestCount", 16);
            p.settings->setDefaultValue("Style/Palette", StylePalette::First);
            p.settings->setDefaultValue("Misc/ToolTipsEnabled", true);
        }

        void App::_modelsInit()
        {
            TLRENDER_P();
            p.filesModel = play::FilesModel::create(_context);

            p.viewportModel = play::ViewportModel::create(_context);
            timeline::BackgroundOptions viewportBackgroundOptions;
            p.settings->getValue("Viewport/Background", viewportBackgroundOptions);
            p.viewportModel->setBackgroundOptions(viewportBackgroundOptions);

            p.colorModel = play::ColorModel::create(_context);
            p.colorModel->setColorConfigOptions(p.options.colorConfigOptions);
            p.colorModel->setLUTOptions(p.options.lutOptions);

            p.audioModel = play::AudioModel::create(_context);
            float volume = 1.F;
            p.settings->getValue("Audio/Volume", volume);
            p.audioModel->setVolume(volume);
            bool mute = false;
            p.settings->getValue("Audio/Mute", mute);
            p.audioModel->setMute(mute);

            p.toolsModel = ToolsModel::create();
        }

        void App::_observersInit()
        {
            TLRENDER_P();
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
                            auto ioOptions = _getIOOptions();
                            ioOptions["Layer"] = string::Format("{0}").arg(value[i]);
                            player->setIOOptions(ioOptions);
                        }
                    }
                });

            p.colorConfigOptionsObserver = observer::ValueObserver<timeline::ColorConfigOptions>::create(
                p.colorModel->observeColorConfigOptions(),
                [this](const timeline::ColorConfigOptions& value)
                {
                    _setColorConfigOptions(value);
                });
            p.lutOptionsObserver = observer::ValueObserver<timeline::LUTOptions>::create(
                p.colorModel->observeLUTOptions(),
                [this](const timeline::LUTOptions& value)
                {
                    _setLUTOptions(value);
                });

            p.volumeObserver = observer::ValueObserver<float>::create(
                p.audioModel->observeVolume(),
                [this](float)
                {
                    _audioUpdate();
                });
            p.muteObserver = observer::ValueObserver<bool>::create(
                p.audioModel->observeMute(),
                [this](bool)
                {
                    _audioUpdate();
                });
            p.syncOffsetObserver = observer::ValueObserver<double>::create(
                p.audioModel->observeSyncOffset(),
                [this](double)
                {
                    _audioUpdate();
                });

            p.settingsObserver = observer::ValueObserver<std::string>::create(
                p.settings->observeValues(),
                [this](const std::string& name)
                {
                    _settingsUpdate(name);
                });
        }

        void App::_inputFilesInit()
        {
            TLRENDER_P();
            if (!p.options.fileName.empty())
            {
                if (!p.options.compareFileName.empty())
                {
                    open(file::Path(p.options.compareFileName));
                    p.filesModel->setCompareOptions(p.options.compareOptions);
                    p.filesModel->setB(0, true);
                }

                open(
                    file::Path(p.options.fileName),
                    file::Path(p.options.audioFileName));

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
        }

        void App::_mainWindowInit()
        {
            TLRENDER_P();
            math::Size2i windowSize = _options.windowSize;
            p.settings->getValue("Window/Size", windowSize);
            setWindowSize(windowSize);
            p.mainWindow = MainWindow::create(
                std::dynamic_pointer_cast<App>(shared_from_this()),
                _context);
            getEventLoop()->addWidget(p.mainWindow);
        }

        io::Options App::_getIOOptions() const
        {
            TLRENDER_P();
            io::Options out;

            {
                int threadCount = 0;
                p.settings->getValue("SequenceIO/ThreadCount", threadCount);
                out["SequenceIO/ThreadCount"] = string::Format("{0}").
                    arg(threadCount);
            }

#if defined(TLRENDER_FFMPEG)
            {
                bool yuvToRGBConversion = false;
                p.settings->getValue("FFmpeg/YUVToRGBConversion", yuvToRGBConversion);
                out["FFmpeg/YUVToRGBConversion"] = string::Format("{0}").
                    arg(yuvToRGBConversion);
            }
            {
                int threadCount = 0;
                p.settings->getValue("FFmpeg/ThreadCount", threadCount);
                out["FFmpeg/ThreadCount"] = string::Format("{0}").
                    arg(threadCount);
            }
#endif // TLRENDER_FFMPEG

#if defined(TLRENDER_USD)
            {
                int renderWidth = 1920;
                p.settings->getValue("USD/renderWidth", renderWidth);
                std::stringstream ss;
                ss << renderWidth;
                out["USD/renderWidth"] = ss.str();
            }
            {
                float complexity = 1.F;
                p.settings->getValue("USD/complexity", complexity);
                std::stringstream ss;
                ss << complexity;
                out["USD/complexity"] = ss.str();
            }
            {
                usd::DrawMode drawMode = usd::DrawMode::First;
                p.settings->getValue("USD/drawMode", drawMode);
                std::stringstream ss;
                ss << drawMode;
                out["USD/drawMode"] = ss.str();
            }
            {
                bool enableLighting = true;
                p.settings->getValue("USD/enableLighting", enableLighting);
                std::stringstream ss;
                ss << enableLighting;
                out["USD/enableLighting"] = ss.str();
            }
            {
                bool sRGB = true;
                p.settings->getValue("USD/sRGB", sRGB);
                std::stringstream ss;
                ss << sRGB;
                out["USD/sRGB"] = ss.str();
            }
            {
                size_t stageCacheCount = 10;
                p.settings->getValue("USD/stageCacheCount", stageCacheCount);
                std::stringstream ss;
                ss << stageCacheCount;
                out["USD/stageCacheCount"] = ss.str();
            }
            {
                size_t diskCacheByteCount = 0;
                p.settings->getValue("USD/diskCacheByteCount", diskCacheByteCount);
                std::stringstream ss;
                ss << diskCacheByteCount;
                out["USD/diskCacheByteCount"] = ss.str();
            }
#endif // TLRENDER_USD

            return out;
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
                activeCount > 0 ? (p.playerCacheOptions.readAhead.value() / static_cast<double>(activeCount)) : 0.0,
                1.0);
        }

        otime::RationalTime App::_getCacheReadBehind() const
        {
            TLRENDER_P();
            const size_t activeCount = p.activeFiles.size();
            return otime::RationalTime(
                activeCount > 0 ? (p.playerCacheOptions.readBehind.value() / static_cast<double>(activeCount)) : 0.0,
                1.0);
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
                        options.ioOptions = _getIOOptions();
                        p.settings->getValue(
                            "FileSequence/MaxDigits",
                            options.pathOptions.maxNumberDigits);
                        auto otioTimeline = items[i]->audioPath.isEmpty() ?
                            timeline::create(items[i]->path, _context, options) :
                            timeline::create(items[i]->path, items[i]->audioPath, _context, options);
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

        void App::_settingsUpdate(const std::string& name)
        {
            TLRENDER_P();
            const auto split = string::split(name, '/');
            if (!split.empty())
            {
                auto ioSystem = _context->getSystem<io::System>();
                const auto& names = ioSystem->getNames();
                const auto i = std::find(names.begin(), names.end(), split[0]);
                if (i != names.end())
                {
                    const auto ioOptions = _getIOOptions();
                    if (ioOptions != p.ioOptions)
                    {
                        p.ioOptions = ioOptions;
                        for (const auto& player : p.players)
                        {
                            if (player)
                            {
                                player->setIOOptions(p.ioOptions);
                            }
                        }
                    }
                }
            }
            {
                int value = 0;
                p.settings->getValue("Cache/Size", value);
                if (value != p.cacheSize)
                {
                    p.cacheSize = value;
                    _cacheUpdate();
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
            auto fileBrowserSystem = _context->getSystem<ui::FileBrowserSystem>();
            {
                std::string path;
                p.settings->getValue("FileBrowser/Path", path);
                fileBrowserSystem->setPath(path);
            }
            {
                ui::FileBrowserOptions options;
                p.settings->getValue("FileBrowser/Options", options);
                fileBrowserSystem->setOptions(options);
            }
            auto recentFilesModel = fileBrowserSystem->getRecentFilesModel();
            {
                int recentFilesMax = 0;
                p.settings->getValue("Files/RecentMax", recentFilesMax);
                recentFilesModel->setRecentMax(recentFilesMax);
            }
            {
                std::vector<std::string> recentFiles;
                p.settings->getValue("Files/Recent", recentFiles);
                std::vector<file::Path> recentPaths;
                for (const auto& recentFile : recentFiles)
                {
                    recentPaths.push_back(file::Path(recentFile));
                }
                recentFilesModel->setRecent(recentPaths);
            }
            {
                bool value = false;
                p.settings->getValue("FileBrowser/NativeFileDialog", value);
                fileBrowserSystem->setNativeFileDialog(value);
            }
            {
                StylePalette value = StylePalette::First;
                p.settings->getValue("Style/Palette", value);
                getStyle()->setColorRoles(getStylePalette(value));
            }
        }

        void App::_cacheUpdate()
        {
            TLRENDER_P();

            // Update the I/O cache.
            auto ioSystem = _context->getSystem<io::System>();
            ioSystem->getCache()->setMax(p.cacheSize * memory::gigabyte);

            // Update inactive players.
            timeline::PlayerCacheOptions cacheOptions;
            cacheOptions.readAhead = time::invalidTime;
            cacheOptions.readBehind = time::invalidTime;
            const auto activePlayers = _getActivePlayers();
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
            const float volume = p.audioModel->getVolume();
            p.settings->setValue("Audio/Volume", volume);
            const bool mute = p.audioModel->isMuted();
            p.settings->setValue("Audio/Mute", mute);
            for (const auto& player : p.players)
            {
                if (player)
                {
                    player->setVolume(volume);
                    player->setMute(mute || p.deviceActive);
                    player->setAudioOffset(p.audioModel->getSyncOffset());
                }
            }
        }
    }
}
