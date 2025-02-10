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
#include <tlPlay/RenderModel.h>
#include <tlPlay/Util.h>
#include <tlPlay/Viewport.h>
#include <tlPlay/ViewportModel.h>

#include <tlTimeline/Util.h>

#if defined(TLRENDER_BMD)
#include <tlDevice/BMDDevicesModel.h>
#include <tlDevice/BMDOutputDevice.h>
#endif // TLRENDER_BMD

#include <tlIO/System.h>

#include <tlCore/FileLogSystem.h>

#include <dtk/ui/FileBrowser.h>
#include <dtk/ui/RecentFilesModel.h>
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
            std::shared_ptr<play::FilesModel> filesModel;
            std::vector<std::shared_ptr<play::FilesModelItem> > files;
            std::vector<std::shared_ptr<play::FilesModelItem> > activeFiles;
            std::shared_ptr<dtk::RecentFilesModel> recentFilesModel;
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
            std::shared_ptr<bmd::DevicesModel> bmdDevicesModel;
            std::shared_ptr<bmd::OutputDevice> bmdOutputDevice;
            dtk::VideoLevels bmdOutputVideoLevels = dtk::VideoLevels::LegalRange;
#endif // TLRENDER_BMD

            std::shared_ptr<dtk::ListObserver<std::shared_ptr<play::FilesModelItem> > > filesObserver;
            std::shared_ptr<dtk::ListObserver<std::shared_ptr<play::FilesModelItem> > > activeObserver;
            std::shared_ptr<dtk::ListObserver<int> > layersObserver;
            std::shared_ptr<dtk::ValueObserver<timeline::CompareTimeMode> > compareTimeObserver;
            std::shared_ptr<dtk::ValueObserver<audio::DeviceID> > audioDeviceObserver;
            std::shared_ptr<dtk::ValueObserver<float> > volumeObserver;
            std::shared_ptr<dtk::ValueObserver<bool> > muteObserver;
            std::shared_ptr<dtk::ListObserver<bool> > channelMuteObserver;
            std::shared_ptr<dtk::ValueObserver<double> > syncOffsetObserver;
            std::shared_ptr<dtk::ValueObserver<std::shared_ptr<dtk::Window> > > closeObserver;
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
            const std::filesystem::path logPath = play::logFileName(appName, appDocsPath);
            const std::filesystem::path settingsPath = play::settingsName(appName, appDocsPath);
            dtk::App::_init(
                context,
                argv,
                appName,
                "Playback application.",
                play::getCmdLineArgs(p.options),
                play::getCmdLineOptions(p.options, logPath, settingsPath));

            p.fileLogSystem = file::FileLogSystem::create(
                _context,
                !p.options.logPath.empty() ? p.options.logPath : logPath);
            _settingsInit(!p.options.settingsPath.empty() ? p.options.settingsPath : settingsPath);
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
        {
            DTK_P();
            if (p.settings)
            {
                nlohmann::json json;
                for (const auto& path : p.recentFilesModel->getRecent())
                {
                    json.push_back(path.u8string());
                }
                p.settings->set("Files/Recent", json);
                p.settings->set("Files/RecentMax", p.recentFilesModel->getRecentMax());

                auto fileBrowserSystem = _context->getSystem<dtk::FileBrowserSystem>();
                p.settings->set("FileBrowser/Path", fileBrowserSystem->getPath().u8string());
                p.settings->setT("FileBrowser/Options", fileBrowserSystem->getOptions());

#if defined(TLRENDER_BMD)
                p.settings->setT("BMD", p.bmdDevicesModel->observeData()->get());
#endif // TLRENDER_BMD

                p.settings->setT("Window/Size", p.mainWindow->getSize());
            }
        }

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
            p.settings->get("FileSequence/MaxDigits", pathOptions.maxNumberDigits);
            for (const auto& i : timeline::getPaths(_context, path, pathOptions))
            {
                auto item = std::make_shared<play::FilesModelItem>();
                item->path = i;
                item->audioPath = audioPath;
                p.filesModel->add(item);
                p.recentFilesModel->addRecent(path.get());
            }
        }

        const std::shared_ptr<play::FilesModel>& App::getFilesModel() const
        {
            return _p->filesModel;
        }

        const std::shared_ptr<dtk::RecentFilesModel>& App::getRecentFilesModel() const
        {
            return _p->recentFilesModel;
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
                if (value)
                {
                    //! \bug macOS does not seem to like having an application with
                    //! normal and fullscreen windows.
                    int secondaryScreen = -1;
#if !defined(__APPLE__)
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
                        std::dynamic_pointer_cast<App>(shared_from_this()),
                        p.mainWindow);
                    addWindow(p.secondaryWindow);
                    if (secondaryScreen != -1)
                    {
                        p.secondaryWindow->setFullScreen(true, secondaryScreen);
                    }
                    p.secondaryWindow->show();
                }
                else
                {
                    removeWindow(p.secondaryWindow);
                    p.secondaryWindow.reset();
                }
            }
        }

#if defined(TLRENDER_BMD)
        const std::shared_ptr<bmd::DevicesModel>& App::getBMDDevicesModel() const
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

        void App::_settingsInit(const std::filesystem::path& path)
        {
            DTK_P();
            p.settings = dtk::Settings::create(
                _context,
                path,
                p.options.resetSettings);
            {
                std::string path;
                dtk::FileBrowserOptions options;
                p.settings->get("FileBrowser/Path", path);
                p.settings->getT("FileBrowser/Options", options);
                auto fileBrowserSystem = _context->getSystem<dtk::FileBrowserSystem>();
                fileBrowserSystem->setPath(path);
                fileBrowserSystem->setOptions(options);
            }
        }

        void App::_modelsInit()
        {
            DTK_P();
            
            p.filesModel = play::FilesModel::create(_context);

            p.recentFilesModel = dtk::RecentFilesModel::create(_context);
            {
                std::vector<std::filesystem::path> recent;
                nlohmann::json json;
                if (p.settings->get("Files/Recent", json))
                {
                    for (auto i = json.begin(); i != json.end(); ++i)
                    {
                        if (i->is_string())
                        {
                            recent.push_back(std::filesystem::u8path(i->get<std::string>()));
                        }
                    }
                }
                p.recentFilesModel->setRecent(recent);
                size_t max = 10;
                p.settings->get("Files/RecentMax", max);
                p.recentFilesModel->setRecentMax(max);
            }

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
            p.bmdDevicesModel = bmd::DevicesModel::create(_context);
            bmd::DevicesModelData data;
            p.settings->getT("BMD", data);
#endif // TLRENDER_BMD
        }

        void App::_observersInit()
        {
            DTK_P();

            p.player = dtk::ObservableValue<std::shared_ptr<timeline::Player> >::create();

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

            p.closeObserver = dtk::ValueObserver<std::shared_ptr<dtk::Window> >::create(
                observeWindowClose(),
                [this](const std::shared_ptr<dtk::Window>& value)
                {
                    if (value && value == _p->secondaryWindow)
                    {
                        _p->secondaryWindowActive->setIfChanged(false);
                        _p->secondaryWindow.reset();
                    }
                    else if (value && value == _p->mainWindow)
                    {
                        removeWindow(_p->secondaryWindow);
                        _p->secondaryWindow.reset();
                    }
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

            dtk::Size2I size(1920, 1080);
            p.settings->getT("Window/Size", size);
            p.mainWindow = MainWindow::create(
                _context,
                std::dynamic_pointer_cast<App>(shared_from_this()),
                size);
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
        }

        io::Options App::_getIOOptions() const
        {
            DTK_P();
            io::Options out;

            double d = 24.0;
            p.settings->get("SequenceIO/DefaultSpeed", d);
            out["SequenceIO/DefaultSpeed"] = dtk::Format("{0}").arg(d);
            int i = 16;
            p.settings->get("SequenceIO/ThreadCount", i);
            out["SequenceIO/ThreadCount"] = dtk::Format("{0}").arg(i);

#if defined(TLRENDER_FFMPEG)
            bool b = false;
            p.settings->get("FFmpeg/YUVToRGBConversion", b);
            out["FFmpeg/YUVToRGBConversion"] = dtk::Format("{0}").arg(b);
            i = 0;
            p.settings->get("FFmpeg/ThreadCount", i);
            out["FFmpeg/ThreadCount"] = dtk::Format("{0}").arg(i);
#endif // TLRENDER_FFMPEG

#if defined(TLRENDER_USD)
            {
                int i = p.options.usdRenderWidth;
                p.settings->get("USD/renderWidth", i);
                out["USD/renderWidth"] = dtk::Format("{0}").arg(i);
            }
            {
                float f = p.options.usdComplexity;
                p.settings->get("USD/complexity", f);
                out["USD/complexity"] = dtk::Format("{0}").arg(f);
            }
            {
                std::string s = usd::to_string(p.options.usdDrawMode);
                p.settings->get("USD/drawMode", s);
                out["USD/drawMode"] = s;
            }
            {
                bool b = p.options.usdEnableLighting;
                p.settings->get("USD/enableLighting", b);
                out["USD/enableLighting"] = dtk::Format("{0}").arg(b);
            }
            {
                bool b = p.options.usdSRGB;
                p.settings->get("USD/sRGB", b);
                out["USD/sRGB"] = dtk::Format("{0}").arg(b);
            }
            {
                size_t s = p.options.usdStageCache;
                p.settings->get("USD/stageCache", s);
                out["USD/stageCacheCount"] = dtk::Format("{0}").arg(b);
            }
            {
                size_t s = p.options.usdDiskCache;
                p.settings->get("USD/diskCache", b);
                out["USD/diskCacheByteCount"] = dtk::Format("{0}").arg(b);
            }
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
                        std::string s;
                        p.settings->get("FileSequence/Audio", s);
                        timeline::from_string(s, options.fileSequenceAudio);
                        p.settings->get("FileSequence/AudioFileName", options.fileSequenceAudioFileName);
                        p.settings->get("FileSequence/AudioDirectory", options.fileSequenceAudioDirectory);
                        p.settings->get("FileSequence/VideoRequestCount", options.videoRequestCount);
                        p.settings->get("FileSequence/AudioRequestCount", options.audioRequestCount);
                        options.ioOptions = _getIOOptions();
                        p.settings->get("FileSequence/MaxDigits", options.pathOptions.maxNumberDigits);
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
                                p.settings->get("Performance/AudioBufferFrameCount", playerOptions.audioBufferFrameCount);
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
            _cacheUpdate();
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

        void App::_cacheUpdate()
        {
            DTK_P();

            auto ioSystem = _context->getSystem<io::System>();
            size_t size = 1;
            p.settings->get("Cache/Size", size);
            ioSystem->getCache()->setMax(size * dtk::gigabyte);

            timeline::PlayerCacheOptions cacheOptions;
            double d = 1.0;
            p.settings->get("Cache/ReadAhead", d);
            cacheOptions.readAhead = OTIO_NS::RationalTime(d, 1.0);
            p.settings->get("Cache/ReadBehind", d);
            cacheOptions.readBehind = OTIO_NS::RationalTime(d, 1.0);
            if (auto player = p.player->get())
            {
                player->setCacheOptions(cacheOptions);
            }
        }

        void App::_viewUpdate(const dtk::V2I& pos, double zoom, bool frame)
        {
            DTK_P();
            float scale = 1.F;
            const dtk::Box2I& g = p.mainWindow->getViewport()->getGeometry();
            if (p.secondaryWindow)
            {
                const dtk::Size2I& secondarySize = p.secondaryWindow->getSize();
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
