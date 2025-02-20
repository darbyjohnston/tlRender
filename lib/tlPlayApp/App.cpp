// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2025 Darby Johnston
// All rights reserved.

#include <tlPlayApp/App.h>

#include <tlPlayApp/MainWindow.h>
#include <tlPlayApp/SecondaryWindow.h>
#include <tlPlayApp/SeparateAudioDialog.h>
#include <tlPlayApp/Tools.h>

#include <tlPlay/App.h>
#include <tlPlay/AudioModel.h>
#include <tlPlay/ColorModel.h>
#include <tlPlay/FilesModel.h>
#include <tlPlay/RecentFilesModel.h>
#include <tlPlay/RenderModel.h>
#include <tlPlay/TimeUnitsModel.h>
#include <tlPlay/Util.h>
#include <tlPlay/Viewport.h>
#include <tlPlay/ViewportModel.h>
#if defined(TLRENDER_BMD)
#include <tlPlay/BMDDevicesModel.h>
#endif // TLRENDER_BMD

#include <tlTimelineUI/ThumbnailSystem.h>

#include <tlTimeline/Util.h>

#if defined(TLRENDER_BMD)
#include <tlDevice/BMDDevicesModel.h>
#include <tlDevice/BMDOutputDevice.h>
#endif // TLRENDER_BMD

#include <tlIO/System.h>

#include <tlCore/FileLogSystem.h>

#include <dtk/ui/FileBrowser.h>
#include <dtk/ui/Settings.h>
#include <dtk/core/Format.h>

#include <filesystem>

namespace tl
{
    namespace play_app
    {
        struct App::Private
        {
            play::Options options;
            std::shared_ptr<file::FileLogSystem> fileLogSystem;
            std::shared_ptr<dtk::Settings> settings;
            std::shared_ptr<play::SettingsModel> settingsModel;
            std::shared_ptr<play::TimeUnitsModel> timeUnitsModel;
            std::shared_ptr<play::FilesModel> filesModel;
            std::vector<std::shared_ptr<play::FilesModelItem> > files;
            std::vector<std::shared_ptr<play::FilesModelItem> > activeFiles;
            std::shared_ptr<play::RecentFilesModel> recentFilesModel;
            std::vector<std::shared_ptr<timeline::Timeline> > timelines;
            std::shared_ptr<dtk::ObservableValue<std::shared_ptr<timeline::Player> > > player;
            std::shared_ptr<play::ColorModel> colorModel;
            std::shared_ptr<play::ViewportModel> viewportModel;
            std::shared_ptr<play::RenderModel> renderModel;
            std::shared_ptr<play::AudioModel> audioModel;
            std::shared_ptr<ToolsModel> toolsModel;

            std::shared_ptr<dtk::ObservableValue<bool> > secondaryWindowActive;
            std::shared_ptr<MainWindow> mainWindow;
            std::shared_ptr<SecondaryWindow> secondaryWindow;
            std::shared_ptr<SeparateAudioDialog> separateAudioDialog;

            bool bmdDeviceActive = false;
#if defined(TLRENDER_BMD)
            std::shared_ptr<play::BMDDevicesModel> bmdDevicesModel;
            std::shared_ptr<bmd::OutputDevice> bmdOutputDevice;
            dtk::VideoLevels bmdOutputVideoLevels = dtk::VideoLevels::LegalRange;
#endif // TLRENDER_BMD

            std::shared_ptr<dtk::ValueObserver<play::CacheOptions> > cacheObserver;
            std::shared_ptr<dtk::ListObserver<std::shared_ptr<play::FilesModelItem> > > filesObserver;
            std::shared_ptr<dtk::ListObserver<std::shared_ptr<play::FilesModelItem> > > activeObserver;
            std::shared_ptr<dtk::ListObserver<int> > layersObserver;
            std::shared_ptr<dtk::ValueObserver<timeline::CompareTimeMode> > compareTimeObserver;
            std::shared_ptr<dtk::ValueObserver<audio::DeviceID> > audioDeviceObserver;
            std::shared_ptr<dtk::ValueObserver<float> > volumeObserver;
            std::shared_ptr<dtk::ValueObserver<bool> > muteObserver;
            std::shared_ptr<dtk::ListObserver<bool> > channelMuteObserver;
            std::shared_ptr<dtk::ValueObserver<double> > syncOffsetObserver;
            std::shared_ptr<dtk::ValueObserver<play::StyleOptions> > styleObserver;
#if defined(TLRENDER_BMD)
            std::shared_ptr<dtk::ValueObserver<bmd::DevicesModelData> > bmdDevicesObserver;
            std::shared_ptr<dtk::ValueObserver<bool> > bmdActiveObserver;
            std::shared_ptr<dtk::ValueObserver<dtk::Size2I> > bmdSizeObserver;
            std::shared_ptr<dtk::ValueObserver<bmd::FrameRate> > bmdFrameRateObserver;
            std::shared_ptr<dtk::ValueObserver<timeline::OCIOOptions> > ocioOptionsObserver;
            std::shared_ptr<dtk::ValueObserver<timeline::LUTOptions> > lutOptionsObserver;
            std::shared_ptr<dtk::ValueObserver<dtk::ImageOptions> > imageOptionsObserver;
            std::shared_ptr<dtk::ValueObserver<timeline::DisplayOptions> > displayOptionsObserver;
            std::shared_ptr<dtk::ValueObserver<timeline::CompareOptions> > compareOptionsObserver;
            std::shared_ptr<dtk::ValueObserver<timeline::BackgroundOptions> > backgroundOptionsObserver;
#endif // TLRENDER_BMD
        };

        void App::_init(
            const std::shared_ptr<dtk::Context>& context,
            std::vector<std::string>& argv)
        {
            DTK_P();
            const std::string appName = "tlplay";
            const std::filesystem::path appDocsPath = play::appDocsPath();
            const std::filesystem::path logFile = play::logFileName(appName, appDocsPath);
            const std::filesystem::path settingsPath = play::settingsName(appName, appDocsPath);
            dtk::App::_init(
                context,
                argv,
                appName,
                "Playback application.",
                play::getCmdLineArgs(p.options),
                play::getCmdLineOptions(p.options, logFile, settingsPath));

            p.fileLogSystem = file::FileLogSystem::create(
                _context,
                !p.options.logFile.empty() ? std::filesystem::u8path(p.options.logFile) : logFile);
            p.settings = dtk::Settings::create(
                _context,
                !p.options.settings.empty() ? std::filesystem::u8path(p.options.settings) : settingsPath,
                p.options.resetSettings);
            _modelsInit();
            _devicesInit();
            _observersInit();
            _inputFilesInit();
            _windowsInit();
        }

        App::App() :
            _p(new Private)
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

        void App::openDialog()
        {
            DTK_P();
            auto fileBrowserSystem = _context->getSystem<dtk::FileBrowserSystem>();
            fileBrowserSystem->open(
                p.mainWindow,
                [this](const std::filesystem::path& value)
                {
                    open(file::Path(value.u8string()));
                },
                p.recentFilesModel);
        }

        void App::openSeparateAudioDialog()
        {
            DTK_P();
            p.separateAudioDialog = SeparateAudioDialog::create(_context);
            p.separateAudioDialog->open(p.mainWindow);
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
            DTK_P();
            file::PathOptions pathOptions;
            pathOptions.maxNumberDigits = p.settingsModel->getFileSequence().maxDigits;
            for (const auto& i : timeline::getPaths(_context, path, pathOptions))
            {
                auto item = std::make_shared<play::FilesModelItem>();
                item->path = i;
                item->audioPath = audioPath;
                p.filesModel->add(item);
                p.recentFilesModel->addRecent(path.get());
            }
        }

        const std::shared_ptr<play::SettingsModel>& App::getSettingsModel() const
        {
            return _p->settingsModel;
        }

        const std::shared_ptr<play::TimeUnitsModel>& App::getTimeUnitsModel() const
        {
            return _p->timeUnitsModel;
        }

        const std::shared_ptr<play::FilesModel>& App::getFilesModel() const
        {
            return _p->filesModel;
        }

        const std::shared_ptr<play::RecentFilesModel>& App::getRecentFilesModel() const
        {
            return _p->recentFilesModel;
        }

        void App::reload()
        {
            DTK_P();
            const auto activeFiles = p.activeFiles;
            const auto files = p.files;
            for (const auto& i : activeFiles)
            {
                const auto j = std::find(p.files.begin(), p.files.end(), i);
                if (j != p.files.end())
                {
                    const size_t index = j - p.files.begin();
                    p.files.erase(j);
                    p.timelines.erase(p.timelines.begin() + index);
                }
            }
            p.activeFiles.clear();
            if (!activeFiles.empty())
            {
                if (auto player = p.player->get())
                {
                    activeFiles.front()->currentTime = player->getCurrentTime();
                }
            }

            auto thumbnailSytem = _context->getSystem<timelineui::ThumbnailSystem>();
            thumbnailSytem->getCache()->clear();

            auto ioSystem = _context->getSystem<io::System>();
            ioSystem->getCache()->clear();

            _filesUpdate(files);
            _activeUpdate(activeFiles);
        }

        std::shared_ptr<dtk::IObservableValue<std::shared_ptr<timeline::Player> > > App::observePlayer() const
        {
            return _p->player;
        }

        const std::shared_ptr<play::ColorModel>& App::getColorModel() const
        {
            return _p->colorModel;
        }

        const std::shared_ptr<play::ViewportModel>& App::getViewportModel() const
        {
            return _p->viewportModel;
        }

        const std::shared_ptr<play::RenderModel>& App::getRenderModel() const
        {
            return _p->renderModel;
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

        std::shared_ptr<dtk::IObservableValue<bool> > App::observeSecondaryWindow() const
        {
            return _p->secondaryWindowActive;
        }

        void App::setSecondaryWindow(bool value)
        {
            DTK_P();
            if (p.secondaryWindowActive->setIfChanged(value))
            {
                if (p.secondaryWindow)
                {
                    removeWindow(p.secondaryWindow);
                    p.secondaryWindow.reset();
                }

                if (value)
                {
                    //! \bug macOS and Windows do not seem to like having an
                    //! application with normal and fullscreen windows?
                    int secondaryScreen = -1;
#if defined(__APPLE__)
#elif defined(_WINDOWS)
#else
                    std::vector<int> screens;
                    for (int i = 0; i < getScreenCount(); ++i)
                    {
                        screens.push_back(i);
                    }
                    auto i = std::find(
                        screens.begin(),
                        screens.end(),
                        p.mainWindow->getScreen());
                    if (i != screens.end())
                    {
                        screens.erase(i);
                    }
                    if (!screens.empty())
                    {
                        secondaryScreen = screens.front();
                    }
#endif // __APPLE__

                    p.secondaryWindow = SecondaryWindow::create(
                        _context,
                        std::dynamic_pointer_cast<App>(shared_from_this()));
                    p.secondaryWindow->setCloseCallback(
                        [this]
                        {
                            DTK_P();
                            p.secondaryWindowActive->setIfChanged(false);
                            removeWindow(p.secondaryWindow);
                            p.secondaryWindow.reset();
                        });
                    addWindow(p.secondaryWindow);
                    if (secondaryScreen != -1)
                    {
                        p.secondaryWindow->setFullScreen(true, secondaryScreen);
                    }
                    p.secondaryWindow->show();
                }
            }
        }

#if defined(TLRENDER_BMD)
        const std::shared_ptr<play::BMDDevicesModel>& App::getBMDDevicesModel() const
        {
            return _p->bmdDevicesModel;
        }

        const std::shared_ptr<bmd::OutputDevice>& App::getBMDOutputDevice() const
        {
            return _p->bmdOutputDevice;
        }
#endif // TLRENDER_BMD

        void App::_tick()
        {
            DTK_P();
            if (auto player = p.player->get())
            {
                player->tick();
            }
#if defined(TLRENDER_BMD)
            if (p.bmdOutputDevice)
            {
                p.bmdOutputDevice->tick();
            }
#endif // TLRENDER_BMD
        }

        void App::_modelsInit()
        {
            DTK_P();

            p.settingsModel = play::SettingsModel::create(_context, p.settings);

            p.timeUnitsModel = play::TimeUnitsModel::create(_context, p.settings);
            
            p.filesModel = play::FilesModel::create(_context);

            p.recentFilesModel = play::RecentFilesModel::create(_context, p.settings);

            p.colorModel = play::ColorModel::create(_context);
            p.colorModel->setOCIOOptions(p.options.ocioOptions);
            p.colorModel->setLUTOptions(p.options.lutOptions);

            p.viewportModel = play::ViewportModel::create(_context, p.settings);

            p.renderModel = play::RenderModel::create(_context, p.settings);

            p.audioModel = play::AudioModel::create(_context, p.settings);

            p.toolsModel = ToolsModel::create();
        }

        void App::_devicesInit()
        {
            DTK_P();
#if defined(TLRENDER_BMD)
            p.bmdOutputDevice = bmd::OutputDevice::create(_context);
            p.bmdDevicesModel = play::BMDDevicesModel::create(_context, p.settings);
#endif // TLRENDER_BMD
        }

        void App::_observersInit()
        {
            DTK_P();

            p.player = dtk::ObservableValue<std::shared_ptr<timeline::Player> >::create();

            p.cacheObserver = dtk::ValueObserver<play::CacheOptions>::create(
                p.settingsModel->observeCache(),
                [this](const play::CacheOptions& value)
                {
                    _cacheUpdate(value);
                });

            p.filesObserver = dtk::ListObserver<std::shared_ptr<play::FilesModelItem> >::create(
                p.filesModel->observeFiles(),
                [this](const std::vector<std::shared_ptr<play::FilesModelItem> >& value)
                {
                    _filesUpdate(value);
                });
            p.activeObserver = dtk::ListObserver<std::shared_ptr<play::FilesModelItem> >::create(
                p.filesModel->observeActive(),
                [this](const std::vector<std::shared_ptr<play::FilesModelItem> >& value)
                {
                    _activeUpdate(value);
                });
            p.layersObserver = dtk::ListObserver<int>::create(
                p.filesModel->observeLayers(),
                [this](const std::vector<int>& value)
                {
                    _layersUpdate(value);
                });
            p.compareTimeObserver = dtk::ValueObserver<timeline::CompareTimeMode>::create(
                p.filesModel->observeCompareTime(),
                [this](timeline::CompareTimeMode value)
                {
                    if (auto player = _p->player->get())
                    {
                        player->setCompareTime(value);
                    }
                });

            p.audioDeviceObserver = dtk::ValueObserver<audio::DeviceID>::create(
                p.audioModel->observeDevice(),
                [this](const audio::DeviceID& value)
                {
                    if (auto player = _p->player->get())
                    {
                        player->setAudioDevice(value);
                    }
                });
            p.volumeObserver = dtk::ValueObserver<float>::create(
                p.audioModel->observeVolume(),
                [this](float)
                {
                    _audioUpdate();
                });
            p.muteObserver = dtk::ValueObserver<bool>::create(
                p.audioModel->observeMute(),
                [this](bool)
                {
                    _audioUpdate();
                });
            p.channelMuteObserver = dtk::ListObserver<bool>::create(
                p.audioModel->observeChannelMute(),
                [this](const std::vector<bool>&)
                {
                    _audioUpdate();
                });
            p.syncOffsetObserver = dtk::ValueObserver<double>::create(
                p.audioModel->observeSyncOffset(),
                [this](double)
                {
                    _audioUpdate();
                });

            p.styleObserver = dtk::ValueObserver<play::StyleOptions>::create(
                p.settingsModel->observeStyle(),
                [this](const play::StyleOptions& value)
                {
                    setColorStyle(value.colorStyle);
                });

#if defined(TLRENDER_BMD)
            p.bmdDevicesObserver = dtk::ValueObserver<bmd::DevicesModelData>::create(
                p.bmdDevicesModel->observeData(),
                [this](const bmd::DevicesModelData& value)
                {
                    DTK_P();
                    bmd::DeviceConfig config;
                    config.deviceIndex = value.deviceIndex - 1;
                    config.displayModeIndex = value.displayModeIndex - 1;
                    config.pixelType = value.pixelTypeIndex >= 0 &&
                        value.pixelTypeIndex < value.pixelTypes.size() ?
                        value.pixelTypes[value.pixelTypeIndex] :
                        bmd::PixelType::None;
                    config.boolOptions = value.boolOptions;
                    p.bmdOutputDevice->setConfig(config);
                    p.bmdOutputDevice->setEnabled(value.deviceEnabled);
                    p.bmdOutputVideoLevels = value.videoLevels;
                    timeline::DisplayOptions displayOptions = p.viewportModel->getDisplayOptions();
                    displayOptions.videoLevels = p.bmdOutputVideoLevels;
                    std::vector<timeline::DisplayOptions> displayOptionsList;
                    p.bmdOutputDevice->setDisplayOptions({ displayOptionsList });
                    p.bmdOutputDevice->setHDR(value.hdrMode, value.hdrData);
                });
            p.bmdActiveObserver = dtk::ValueObserver<bool>::create(
                p.bmdOutputDevice->observeActive(),
                [this](bool value)
                {
                    _p->bmdDeviceActive = value;
                    _audioUpdate();
                });
            p.bmdSizeObserver = dtk::ValueObserver<dtk::Size2I>::create(
                p.bmdOutputDevice->observeSize(),
                [this](const dtk::Size2I& value)
                {
                    //std::cout << "output device size: " << value << std::endl;
                });
            p.bmdFrameRateObserver = dtk::ValueObserver<bmd::FrameRate>::create(
                p.bmdOutputDevice->observeFrameRate(),
                [this](const bmd::FrameRate& value)
                {
                    //std::cout << "output device frame rate: " <<
                    //    value.num << "/" <<
                    //    value.den <<
                    //    std::endl;
                });

            p.ocioOptionsObserver = dtk::ValueObserver<timeline::OCIOOptions>::create(
                p.colorModel->observeOCIOOptions(),
                [this](const timeline::OCIOOptions& value)
                {
                    _p->bmdOutputDevice->setOCIOOptions(value);
                });
            p.lutOptionsObserver = dtk::ValueObserver<timeline::LUTOptions>::create(
                p.colorModel->observeLUTOptions(),
                [this](const timeline::LUTOptions& value)
                {
                    _p->bmdOutputDevice->setLUTOptions(value);
                });
            p.imageOptionsObserver = dtk::ValueObserver<dtk::ImageOptions>::create(
                p.renderModel->observeImageOptions(),
                [this](const dtk::ImageOptions& value)
                {
                    _p->bmdOutputDevice->setImageOptions({ value });
                });
            p.displayOptionsObserver = dtk::ValueObserver<timeline::DisplayOptions>::create(
                p.viewportModel->observeDisplayOptions(),
                [this](const timeline::DisplayOptions& value)
                {
                    timeline::DisplayOptions tmp = value;
                    tmp.videoLevels = _p->bmdOutputVideoLevels;
                    _p->bmdOutputDevice->setDisplayOptions({ tmp });
                });

            p.compareOptionsObserver = dtk::ValueObserver<timeline::CompareOptions>::create(
                p.filesModel->observeCompareOptions(),
                [this](const timeline::CompareOptions& value)
                {
                    _p->bmdOutputDevice->setCompareOptions(value);
                });

            p.backgroundOptionsObserver = dtk::ValueObserver<timeline::BackgroundOptions>::create(
                p.viewportModel->observeBackgroundOptions(),
                [this](const timeline::BackgroundOptions& value)
                {
                    _p->bmdOutputDevice->setBackgroundOptions(value);
                });
#endif // TLRENDER_BMD
        }

        void App::_inputFilesInit()
        {
            DTK_P();
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
        }

        void App::_windowsInit()
        {
            DTK_P();

            p.secondaryWindowActive = dtk::ObservableValue<bool>::create(false);

            p.mainWindow = MainWindow::create(
                _context,
                std::dynamic_pointer_cast<App>(shared_from_this()));
            addWindow(p.mainWindow);
            p.mainWindow->show();

            p.mainWindow->getViewport()->setViewPosAndZoomCallback(
                [this](const dtk::V2I& pos, double zoom)
                {
                    _viewUpdate(
                        pos,
                        zoom,
                        _p->mainWindow->getViewport()->hasFrameView());
                });
            p.mainWindow->getViewport()->setFrameViewCallback(
                [this](bool value)
                {
                    _viewUpdate(
                        _p->mainWindow->getViewport()->getViewPos(),
                        _p->mainWindow->getViewport()->getViewZoom(),
                        value);
                });
            p.mainWindow->setCloseCallback(
                [this]
                {
                    DTK_P();
                    if (p.secondaryWindow)
                    {
                        removeWindow(p.secondaryWindow);
                        p.secondaryWindow.reset();
                    }
                });
        }

        io::Options App::_getIOOptions() const
        {
            DTK_P();
            io::Options out;
            out = io::merge(out, io::getOptions(p.settingsModel->getSequenceIO()));
    #if defined(TLRENDER_FFMPEG)
            out = io::merge(out, ffmpeg::getOptions(p.settingsModel->getFFmpeg()));
#endif // TLRENDER_FFMPEG
#if defined(TLRENDER_USD)
            out = io::merge(out, usd::getOptions(p.settingsModel->getUSD()));
#endif // TLRENDER_USD
            return out;
        }

        void App::_filesUpdate(const std::vector<std::shared_ptr<play::FilesModelItem> >& files)
        {
            DTK_P();

            std::vector<std::shared_ptr<timeline::Timeline> > timelines(files.size());
            for (size_t i = 0; i < files.size(); ++i)
            {
                const auto j = std::find(p.files.begin(), p.files.end(), files[i]);
                if (j != p.files.end())
                {
                    timelines[i] = p.timelines[j - p.files.begin()];
                }
            }

            for (size_t i = 0; i < files.size(); ++i)
            {
                if (!timelines[i])
                {
                    try
                    {
                        timeline::Options options;
                        const play::FileSequenceOptions fileSequence = p.settingsModel->getFileSequence();
                        options.fileSequenceAudio = fileSequence.audio;
                        options.fileSequenceAudioFileName = fileSequence.audioFileName;
                        options.fileSequenceAudioDirectory = fileSequence.audioDirectory;
                        const play::PerformanceOptions performance = p.settingsModel->getPerformance();
                        options.videoRequestCount = performance.videoRequestCount;
                        options.audioRequestCount = performance.audioRequestCount;
                        options.ioOptions = _getIOOptions();
                        options.pathOptions.maxNumberDigits = fileSequence.maxDigits;
                        auto otioTimeline = files[i]->audioPath.isEmpty() ?
                            timeline::create(_context, files[i]->path, options) :
                            timeline::create(_context, files[i]->path, files[i]->audioPath, options);
                        timelines[i] = timeline::Timeline::create(_context, otioTimeline, options);
                        for (const auto& video : timelines[i]->getIOInfo().video)
                        {
                            files[i]->videoLayers.push_back(video.name);
                        }
                    }
                    catch (const std::exception& e)
                    {
                        _context->log("tl::play_app::App", e.what(), dtk::LogType::Error);
                    }
                }
            }

            p.files = files;
            p.timelines = timelines;
        }

        void App::_activeUpdate(const std::vector<std::shared_ptr<play::FilesModelItem> >& activeFiles)
        {
            DTK_P();

            if (!p.activeFiles.empty())
            {
                if (auto player = p.player->get())
                {
                    p.activeFiles.front()->currentTime = player->getCurrentTime();
                }
            }

            std::shared_ptr<timeline::Player> player;
            if (!activeFiles.empty())
            {
                if (!p.activeFiles.empty() && activeFiles[0] == p.activeFiles[0])
                {
                    player = p.player->get();
                }
                else
                {
                    if (auto player = p.player->get())
                    {
                        player->setAudioDevice(audio::DeviceID());
                    }
                    auto i = std::find(p.files.begin(), p.files.end(), activeFiles[0]);
                    if (i != p.files.end())
                    {
                        if (auto timeline = p.timelines[i - p.files.begin()])
                        {
                            try
                            {
                                timeline::PlayerOptions playerOptions;
                                playerOptions.audioDevice = p.audioModel->getDevice();
                                playerOptions.cache.readAhead = time::invalidTime;
                                playerOptions.cache.readBehind = time::invalidTime;
                                playerOptions.audioBufferFrameCount = p.settingsModel->getPerformance().audioBufferFrameCount;
                                player = timeline::Player::create(_context, timeline, playerOptions);
                            }
                            catch (const std::exception& e)
                            {
                                _context->log("tl::play_app::App", e.what(), dtk::LogType::Error);
                            }
                        }
                    }
                }
            }
            if (player)
            {
                const OTIO_NS::RationalTime currentTime = activeFiles.front()->currentTime;
                if (!currentTime.strictly_equal(time::invalidTime))
                {
                    player->seek(currentTime);
                }
                std::vector<std::shared_ptr<timeline::Timeline> > compare;
                for (size_t i = 1; i < activeFiles.size(); ++i)
                {
                    auto j = std::find(p.files.begin(), p.files.end(), activeFiles[i]);
                    if (j != p.files.end())
                    {
                        auto timeline = p.timelines[j - p.files.begin()];
                        compare.push_back(timeline);
                    }
                }
                player->setCompare(compare);
                player->setCompareTime(p.filesModel->getCompareTime());
            }

            p.activeFiles = activeFiles;
            p.player->setIfChanged(player);
#if defined(TLRENDER_BMD)
            p.bmdOutputDevice->setPlayer(player);
#endif // TLRENDER_BMD

            _layersUpdate(p.filesModel->observeLayers()->get());
            _cacheUpdate(p.settingsModel->getCache());
            _audioUpdate();
        }

        void App::_layersUpdate(const std::vector<int>& value)
        {
            DTK_P();
            if (auto player = p.player->get())
            {
                int videoLayer = 0;
                std::vector<int> compareVideoLayers;
                if (!value.empty() && value.size() == p.files.size() && !p.activeFiles.empty())
                {
                    auto i = std::find(p.files.begin(), p.files.end(), p.activeFiles.front());
                    if (i != p.files.end())
                    {
                        videoLayer = value[i - p.files.begin()];
                    }
                    for (size_t j = 1; j < p.activeFiles.size(); ++j)
                    {
                        i = std::find(p.files.begin(), p.files.end(), p.activeFiles[j]);
                        if (i != p.files.end())
                        {
                            compareVideoLayers.push_back(value[i - p.files.begin()]);
                        }
                    }
                }
                player->setVideoLayer(videoLayer);
                player->setCompareVideoLayers(compareVideoLayers);
            }
        }

        void App::_cacheUpdate(const play::CacheOptions& options)
        {
            DTK_P();

            auto ioSystem = _context->getSystem<io::System>();
            ioSystem->getCache()->setMax(options.sizeGB * dtk::gigabyte);

            timeline::PlayerCacheOptions cacheOptions;
            cacheOptions.readAhead = OTIO_NS::RationalTime(options.readAhead, 1.0);
            cacheOptions.readBehind = OTIO_NS::RationalTime(options.readBehind, 1.0);
            if (auto player = p.player->get())
            {
                player->setCacheOptions(cacheOptions);
            }
        }

        void App::_viewUpdate(const dtk::V2I& pos, double zoom, bool frame)
        {
            DTK_P();
            const dtk::Box2I& g = p.mainWindow->getViewport()->getGeometry();
            float scale = 1.F;
            if (p.secondaryWindow)
            {
                const dtk::Size2I& secondarySize = p.secondaryWindow->getViewport()->getGeometry().size();
                if (g.isValid() && secondarySize.isValid())
                {
                    scale = secondarySize.w / static_cast<float>(g.w());
                }
                p.secondaryWindow->setView(pos * scale, zoom * scale, frame);
            }
#if defined(TLRENDER_BMD)
            scale = 1.F;
            const dtk::Size2I& bmdSize = p.bmdOutputDevice->getSize();
            if (g.isValid() && bmdSize.isValid())
            {
                scale = bmdSize.w / static_cast<float>(g.w());
            }
            p.bmdOutputDevice->setView(pos * scale, zoom * scale, frame);
#endif // TLRENDER_BMD
        }

        void App::_audioUpdate()
        {
            DTK_P();
            const float volume = p.audioModel->getVolume();
            const bool mute = p.audioModel->isMuted();
            const std::vector<bool> channelMute = p.audioModel->getChannelMute();
            const double audioOffset = p.audioModel->getSyncOffset();
            if (auto player = p.player->get())
            {
                player->setVolume(volume);
                player->setMute(mute || p.bmdDeviceActive);
                player->setChannelMute(channelMute);
                player->setAudioOffset(audioOffset);
            }
#if defined(TLRENDER_BMD)
            p.bmdOutputDevice->setVolume(volume);
            p.bmdOutputDevice->setMute(mute);
            p.bmdOutputDevice->setChannelMute(channelMute);
            p.bmdOutputDevice->setAudioOffset(audioOffset);
#endif // TLRENDER_BMD
        }
    }
}
