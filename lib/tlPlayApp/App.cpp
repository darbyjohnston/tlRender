// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2025 Darby Johnston
// All rights reserved.

#include <tlPlayApp/App.h>

#include <tlPlayApp/MainWindow.h>
#include <tlPlayApp/SecondaryWindow.h>
#include <tlPlayApp/SeparateAudioDialog.h>
#include <tlPlayApp/Style.h>
#include <tlPlayApp/Tools.h>

#include <tlPlay/App.h>
#include <tlPlay/AudioModel.h>
#include <tlPlay/ColorModel.h>
#include <tlPlay/FilesModel.h>
#include <tlPlay/RenderModel.h>
#include <tlPlay/Settings.h>
#include <tlPlay/Util.h>
#include <tlPlay/Viewport.h>
#include <tlPlay/ViewportModel.h>

#include <tlUI/FileBrowser.h>
#include <tlUI/RecentFilesModel.h>

#include <tlTimeline/Util.h>

#if defined(TLRENDER_BMD)
#include <tlDevice/BMDDevicesModel.h>
#include <tlDevice/BMDOutputDevice.h>
#endif // TLRENDER_BMD

#include <tlIO/System.h>

#include <tlCore/FileLogSystem.h>

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
            std::string settingsFileName;
            std::shared_ptr<play::Settings> settings;
            std::shared_ptr<play::FilesModel> filesModel;
            std::vector<std::shared_ptr<play::FilesModelItem> > files;
            std::vector<std::shared_ptr<play::FilesModelItem> > activeFiles;
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

            std::shared_ptr<dtk::ValueObserver<std::string> > settingsObserver;
            std::shared_ptr<dtk::ListObserver<std::shared_ptr<play::FilesModelItem> > > filesObserver;
            std::shared_ptr<dtk::ListObserver<std::shared_ptr<play::FilesModelItem> > > activeObserver;
            std::shared_ptr<dtk::ListObserver<int> > layersObserver;
            std::shared_ptr<dtk::ValueObserver<timeline::CompareTimeMode> > compareTimeObserver;
            std::shared_ptr<dtk::ValueObserver<size_t> > recentFilesMaxObserver;
            std::shared_ptr<dtk::ListObserver<file::Path> > recentFilesObserver;
            std::shared_ptr<dtk::ValueObserver<bool> > mainWindowObserver;
            std::shared_ptr<dtk::ValueObserver<bool> > secondaryWindowObserver;
            std::shared_ptr<dtk::ValueObserver<audio::DeviceID> > audioDeviceObserver;
            std::shared_ptr<dtk::ValueObserver<float> > volumeObserver;
            std::shared_ptr<dtk::ValueObserver<bool> > muteObserver;
            std::shared_ptr<dtk::ListObserver<bool> > channelMuteObserver;
            std::shared_ptr<dtk::ValueObserver<double> > syncOffsetObserver;
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
            const std::vector<std::string>& argv)
        {
            DTK_P();
            const std::string appName = "tlplay";
            const std::string appDocsPath = play::appDocsPath();
            const std::string logFileName = play::logFileName(appName, appDocsPath);
            const std::string settingsFileName = play::settingsName(appName, appDocsPath);
            ui_app::App::_init(
                context,
                argv,
                appName,
                "Playback application.",
                play::getCmdLineArgs(p.options),
                play::getCmdLineOptions(p.options, logFileName, settingsFileName));
            const int exitCode = getExit();
            if (exitCode != 0)
            {
                exit(exitCode);
                return;
            }

            _fileLogInit(logFileName);
            _settingsInit(settingsFileName);
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
                auto fileBrowserSystem = _context->getSystem<ui::FileBrowserSystem>();
                p.settings->setValue("FileBrowser/Path", fileBrowserSystem->getPath());
                p.settings->setValue("FileBrowser/Options", fileBrowserSystem->getOptions());
            }
        }

        std::shared_ptr<App> App::create(
            const std::shared_ptr<dtk::Context>& context,
            const std::vector<std::string>& argv)
        {
            auto out = std::shared_ptr<App>(new App);
            out->_init(context, argv);
            return out;
        }

        void App::openDialog()
        {
            DTK_P();
            auto fileBrowserSystem = _context->getSystem<ui::FileBrowserSystem>();
            fileBrowserSystem->open(
                p.mainWindow,
                [this](const file::FileInfo& value)
                {
                    open(value.getPath());
                });
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
            pathOptions.maxNumberDigits = p.settings->getValue<size_t>("FileSequence/MaxDigits");
            for (const auto& i : timeline::getPaths(_context, path, pathOptions))
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

                    p.secondaryWindowObserver = dtk::ValueObserver<bool>::create(
                        p.secondaryWindow->observeClose(),
                        [this](bool value)
                        {
                            if (value)
                            {
                                _p->secondaryWindowActive->setIfChanged(false);
                                _p->secondaryWindow.reset();
                                _p->secondaryWindowObserver.reset();
                            }
                        });
                }
                else
                {
                    removeWindow(p.secondaryWindow);
                    p.secondaryWindow.reset();
                    p.secondaryWindowObserver.reset();
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

        void App::_fileLogInit(const std::string& logFileName)
        {
            DTK_P();
            std::string logFileName2 = logFileName;
            if (!p.options.logFileName.empty())
            {
                logFileName2 = p.options.logFileName;
            }
            p.fileLogSystem = file::FileLogSystem::create(_context, logFileName2);
        }

        void App::_settingsInit(const std::string& settingsFileName)
        {
            DTK_P();
            if (!p.options.settingsFileName.empty())
            {
                p.settingsFileName = p.options.settingsFileName;
            }
            else
            {
                p.settingsFileName = settingsFileName;
            }
            p.settings = play::Settings::create(
                _context,
                p.settingsFileName,
                p.options.resetSettings);

            p.settings->setDefaultValue("Files/RecentMax", 10);

            p.settings->setDefaultValue("Window/Size", dtk::Size2I(1920, 1080));

            p.settings->setDefaultValue("Cache/Size", 1);
            p.settings->setDefaultValue("Cache/ReadAhead", 2.0);
            p.settings->setDefaultValue("Cache/ReadBehind", 0.5);

            p.settings->setDefaultValue("FileSequence/Audio",
                timeline::FileSequenceAudio::BaseName);
            p.settings->setDefaultValue("FileSequence/AudioFileName", std::string());
            p.settings->setDefaultValue("FileSequence/AudioDirectory", std::string());
            p.settings->setDefaultValue("FileSequence/MaxDigits", 9);

            p.settings->setDefaultValue("SequenceIO/DefaultSpeed", 24.0);
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

#if defined(TLRENDER_BMD)
            bmd::DevicesModelData bmdDevicesModelData;
            p.settings->setDefaultValue("BMD/DeviceIndex", bmdDevicesModelData.deviceIndex);
            p.settings->setDefaultValue("BMD/DisplayModeIndex", bmdDevicesModelData.displayModeIndex);
            p.settings->setDefaultValue("BMD/PixelTypeIndex", bmdDevicesModelData.pixelTypeIndex);
            p.settings->setDefaultValue("BMD/DeviceEnabled", bmdDevicesModelData.deviceEnabled);
            const auto i = bmdDevicesModelData.boolOptions.find(bmd::Option::_444SDIVideoOutput);
            p.settings->setDefaultValue("BMD/444SDIVideoOutput", i != bmdDevicesModelData.boolOptions.end() ? i->second : false);
            p.settings->setDefaultValue("BMD/HDRMode", bmdDevicesModelData.hdrMode);
            p.settings->setDefaultValue("BMD/HDRData", bmdDevicesModelData.hdrData);
#endif // TLRENDER_BMD

            p.settings->setDefaultValue("FileBrowser/NativeFileDialog", true);
            p.settings->setDefaultValue("FileBrowser/Path", std::filesystem::current_path().u8string());
            p.settings->setDefaultValue("FileBrowser/Options", ui::FileBrowserOptions());

            p.settings->setDefaultValue("Performance/AudioBufferFrameCount",
                timeline::PlayerOptions().audioBufferFrameCount);
            p.settings->setDefaultValue("Performance/VideoRequestCount", 16);
            p.settings->setDefaultValue("Performance/AudioRequestCount", 16);

            p.settings->setDefaultValue("OpenGL/ShareContexts", true);

            p.settings->setDefaultValue("Style/Palette", StylePalette::First);

            p.settings->setDefaultValue("Misc/ToolTipsEnabled", true);
        }

        void App::_modelsInit()
        {
            DTK_P();
            
            p.filesModel = play::FilesModel::create(_context);

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
            p.bmdDevicesModel->setDeviceIndex(
                p.settings->getValue<int>("BMD/DeviceIndex"));
            p.bmdDevicesModel->setDisplayModeIndex(
                p.settings->getValue<int>("BMD/DisplayModeIndex"));
            p.bmdDevicesModel->setPixelTypeIndex(
                p.settings->getValue<int>("BMD/PixelTypeIndex"));
            p.bmdDevicesModel->setDeviceEnabled(
                p.settings->getValue<bool>("BMD/DeviceEnabled"));
            bmd::BoolOptions deviceBoolOptions;
            deviceBoolOptions[bmd::Option::_444SDIVideoOutput] =
                p.settings->getValue<bool>("BMD/444SDIVideoOutput");
            p.bmdDevicesModel->setBoolOptions(deviceBoolOptions);
            p.bmdDevicesModel->setHDRMode(
                p.settings->getValue<bmd::HDRMode>("BMD/HDRMode"));
            p.bmdDevicesModel->setHDRData(
                p.settings->getValue<image::HDRData>("BMD/HDRData"));
#endif // TLRENDER_BMD
        }

        void App::_observersInit()
        {
            DTK_P();

            p.player = dtk::ObservableValue<std::shared_ptr<timeline::Player> >::create();

            p.settingsObserver = dtk::ValueObserver<std::string>::create(
                p.settings->observeValues(),
                [this](const std::string& name)
                {
                    _settingsUpdate(name);
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

            auto fileBrowserSystem = _context->getSystem<ui::FileBrowserSystem>();
            auto recentFilesModel = fileBrowserSystem->getRecentFilesModel();
            p.recentFilesMaxObserver = dtk::ValueObserver<size_t>::create(
                recentFilesModel->observeRecentMax(),
                [this](size_t value)
                {
                    _p->settings->setValue("Files/RecentMax", value);
                });
            p.recentFilesObserver = dtk::ListObserver<file::Path>::create(
                recentFilesModel->observeRecent(),
                [this](const std::vector<file::Path>& value)
                {
                    std::vector<std::string> fileNames;
                    for (const auto& i : value)
                    {
                        fileNames.push_back(i.get());
                    }
                    _p->settings->setValue("Files/Recent", fileNames);
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

                    p.settings->setValue("BMD/DeviceIndex", value.deviceIndex);
                    p.settings->setValue("BMD/DisplayModeIndex", value.displayModeIndex);
                    p.settings->setValue("BMD/PixelTypeIndex", value.pixelTypeIndex);
                    p.settings->setValue("BMD/DeviceEnabled", value.deviceEnabled);
                    const auto i = value.boolOptions.find(bmd::Option::_444SDIVideoOutput);
                    p.settings->setValue("BMD/444SDIVideoOutput", i != value.boolOptions.end() ? i->second : false);
                    p.settings->setValue("BMD/HDRMode", value.hdrMode);
                    p.settings->setValue("BMD/HDRData", value.hdrData);
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
            p.mainWindow->setWindowSize(
                _uiOptions.windowSize.isValid() ?
                _uiOptions.windowSize :
                p.settings->getValue<dtk::Size2I>("Window/Size"));
            p.mainWindow->setFullScreen(_uiOptions.fullscreen);
            p.mainWindow->show();

            p.mainWindowObserver = dtk::ValueObserver<bool>::create(
                p.mainWindow->observeClose(),
                [this](bool value)
                {
                    if (value)
                    {
                        removeWindow(_p->secondaryWindow);
                        _p->secondaryWindow.reset();
                        _p->secondaryWindowObserver.reset();
                    }
                });

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

            out["SequenceIO/DefaultSpeed"] = dtk::Format("{0}").
                arg(p.settings->getValue<double>("SequenceIO/DefaultSpeed"));
            out["SequenceIO/ThreadCount"] = dtk::Format("{0}").
                arg(p.settings->getValue<int>("SequenceIO/ThreadCount"));

#if defined(TLRENDER_FFMPEG)
            out["FFmpeg/YUVToRGBConversion"] = dtk::Format("{0}").
                arg(p.settings->getValue<bool>("FFmpeg/YUVToRGBConversion"));
            out["FFmpeg/ThreadCount"] = dtk::Format("{0}").
                arg(p.settings->getValue<int>("FFmpeg/ThreadCount"));
#endif // TLRENDER_FFMPEG

#if defined(TLRENDER_USD)
            {
                std::stringstream ss;
                ss << p.settings->getValue<int>("USD/renderWidth");
                out["USD/renderWidth"] = ss.str();
            }
            {
                std::stringstream ss;
                ss << p.settings->getValue<float>("USD/complexity");
                out["USD/complexity"] = ss.str();
            }
            {
                std::stringstream ss;
                ss << p.settings->getValue<usd::DrawMode>("USD/drawMode");
                out["USD/drawMode"] = ss.str();
            }
            {
                std::stringstream ss;
                ss << p.settings->getValue<bool>("USD/enableLighting");
                out["USD/enableLighting"] = ss.str();
            }
            {
                std::stringstream ss;
                ss << p.settings->getValue<bool>("USD/sRGB");
                out["USD/sRGB"] = ss.str();
            }
            {
                std::stringstream ss;
                ss << p.settings->getValue<size_t>("USD/stageCacheCount");
                out["USD/stageCacheCount"] = ss.str();
            }
            {
                std::stringstream ss;
                ss << p.settings->getValue<size_t>("USD/diskCacheByteCount");
                out["USD/diskCacheByteCount"] = ss.str();
            }
#endif // TLRENDER_USD

            return out;
        }

        void App::_settingsUpdate(const std::string& name)
        {
            DTK_P();
            const auto split = dtk::split(name, '/');
            if (!split.empty() || name.empty())
            {
                auto ioSystem = _context->getSystem<io::System>();
                const auto& names = ioSystem->getNames();
                bool match = false;
                if (!split.empty())
                {
                    match = std::find(names.begin(), names.end(), split[0]) != names.end();
                }
                if (match || name.empty())
                {
                    const auto ioOptions = _getIOOptions();
                    if (auto player = p.player->get())
                    {
                        player->setIOOptions(ioOptions);
                    }
                }
            }
            if ("Cache/Size" == name ||
                "Cache/ReadAhead" == name ||
                "Cache/ReadBehind" == name ||
                name.empty())
            {
                _cacheUpdate();
            }
            if ("FileBrowser/Path" == name || name.empty())
            {
                auto fileBrowserSystem = _context->getSystem<ui::FileBrowserSystem>();
                fileBrowserSystem->setPath(
                    p.settings->getValue<std::string>("FileBrowser/Path"));
            }
            if ("FileBrowser/Options" == name || name.empty())
            {
                auto fileBrowserSystem = _context->getSystem<ui::FileBrowserSystem>();
                auto options = p.settings->getValue<ui::FileBrowserOptions>("FileBrowser/Options");
                fileBrowserSystem->setOptions(options);
            }
            if ("FileBrowser/NativeFileDialog" == name || name.empty())
            {
                auto fileBrowserSystem = _context->getSystem<ui::FileBrowserSystem>();
                fileBrowserSystem->setNativeFileDialog(
                    p.settings->getValue<bool>("FileBrowser/NativeFileDialog"));
            }
            if ("Files/RecentMax" == name || name.empty())
            {
                auto fileBrowserSystem = _context->getSystem<ui::FileBrowserSystem>();
                auto recentFilesModel = fileBrowserSystem->getRecentFilesModel();
                recentFilesModel->setRecentMax(
                    p.settings->getValue<int>("Files/RecentMax"));
            }
            if ("Files/Recent" == name || name.empty())
            {
                std::vector<file::Path> recentPaths;
                for (const auto& recentFile :
                    p.settings->getValue<std::vector<std::string> >("Files/Recent"))
                {
                    recentPaths.push_back(file::Path(recentFile));
                }
                auto fileBrowserSystem = _context->getSystem<ui::FileBrowserSystem>();
                auto recentFilesModel = fileBrowserSystem->getRecentFilesModel();
                recentFilesModel->setRecent(recentPaths);
            }
            if ("Style/Palette" == name || name.empty())
            {
                getStyle()->setColorRoles(getStylePalette(
                    p.settings->getValue<StylePalette>("Style/Palette")));
            }
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
                        options.fileSequenceAudio =
                            p.settings->getValue<timeline::FileSequenceAudio>("FileSequence/Audio");
                        options.fileSequenceAudioFileName =
                            p.settings->getValue<std::string>("FileSequence/AudioFileName");
                        options.fileSequenceAudioDirectory =
                            p.settings->getValue<std::string>("FileSequence/AudioDirectory");
                        options.videoRequestCount =
                            p.settings->getValue<size_t>("Performance/VideoRequestCount");
                        options.audioRequestCount =
                            p.settings->getValue<size_t>("Performance/AudioRequestCount");
                        options.ioOptions = _getIOOptions();
                        options.pathOptions.maxNumberDigits =
                            p.settings->getValue<size_t>("FileSequence/MaxDigits");
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
                        _log(e.what(), dtk::LogType::Error);
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
                                playerOptions.audioBufferFrameCount =
                                    p.settings->getValue<size_t>("Performance/AudioBufferFrameCount");
                                player = timeline::Player::create(_context, timeline, playerOptions);
                            }
                            catch (const std::exception& e)
                            {
                                _log(e.what(), dtk::LogType::Error);
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
            ioSystem->getCache()->setMax(
                p.settings->getValue<size_t>("Cache/Size") * dtk::gigabyte);

            timeline::PlayerCacheOptions cacheOptions;
            cacheOptions.readAhead = OTIO_NS::RationalTime(
                p.settings->getValue<double>("Cache/ReadAhead"),
                1.0);
            cacheOptions.readBehind = OTIO_NS::RationalTime(
                p.settings->getValue<double>("Cache/ReadBehind"),
                1.0);
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
                const dtk::Size2I& secondarySize = p.secondaryWindow->getWindowSize();
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
