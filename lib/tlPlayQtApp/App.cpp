// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2023 Darby Johnston
// All rights reserved.

#include <tlPlayQtApp/App.h>

#include <tlPlayQtApp/MainWindow.h>
#include <tlPlayQtApp/OpenSeparateAudioDialog.h>
#include <tlPlayQtApp/SecondaryWindow.h>
#if defined(TLRENDER_BMD)
#include <tlPlayQtApp/BMDDevicesModel.h>
#endif // TLRENDER_BMD

#include <tlPlay/Settings.h>

#include <tlQtWidget/Init.h>
#include <tlQtWidget/FileBrowserSystem.h>
#include <tlQtWidget/Style.h>
#include <tlQtWidget/TimelineViewport.h>
#include <tlQtWidget/Util.h>

#include <tlQt/ContextObject.h>
#include <tlQt/MetaTypes.h>
#include <tlQt/TimeObject.h>
#include <tlQt/TimelinePlayer.h>
#include <tlQt/ToolTipsFilter.h>
#if defined(TLRENDER_BMD)
#include <tlQt/BMDOutputDevice.h>
#endif // TLRENDER_BMD

#include <tlUI/RecentFilesModel.h>

#include <tlPlay/App.h>
#include <tlPlay/AudioModel.h>
#include <tlPlay/ColorModel.h>
#include <tlPlay/FilesModel.h>
#include <tlPlay/ViewportModel.h>
#include <tlPlay/Util.h>

#include <tlTimeline/Util.h>

#include <tlIO/System.h>
#if defined(TLRENDER_USD)
#include <tlIO/USD.h>
#endif // TLRENDER_USD

#include <tlCore/AudioSystem.h>
#include <tlCore/FileLogSystem.h>
#include <tlCore/Math.h>
#include <tlCore/StringFormat.h>
#include <tlCore/Time.h>

#include <QPointer>
#include <QScreen>

namespace tl
{
    namespace play_qt
    {
        struct App::Private
        {
            play::Options options;
            QScopedPointer<qt::ContextObject> contextObject;
            std::shared_ptr<file::FileLogSystem> fileLogSystem;
            std::string settingsFileName;
            std::shared_ptr<play::Settings> settings;
            std::shared_ptr<timeline::TimeUnitsModel> timeUnitsModel;
            QScopedPointer<qt::TimeObject> timeObject;
            std::shared_ptr<play::FilesModel> filesModel;
            std::vector<std::shared_ptr<play::FilesModelItem> > files;
            std::vector<std::shared_ptr<play::FilesModelItem> > activeFiles;
            QVector<QSharedPointer<qt::TimelinePlayer> > players;
            std::shared_ptr<ui::RecentFilesModel> recentFilesModel;
            std::shared_ptr<play::ViewportModel> viewportModel;
            std::shared_ptr<play::ColorModel> colorModel;
            audio::Info audioInfo;
            std::shared_ptr<play::AudioModel> audioModel;
            QScopedPointer<qt::ToolTipsFilter> toolTipsFilter;
            QScopedPointer<MainWindow> mainWindow;
            QScopedPointer<SecondaryWindow> secondaryWindow;
            bool bmdDeviceActive = false;
#if defined(TLRENDER_BMD)
            QScopedPointer<qt::BMDOutputDevice> bmdOutputDevice;
            std::shared_ptr<BMDDevicesModel> bmdDevicesModel;
#endif // TLRENDER_BMD

            std::shared_ptr<observer::ValueObserver<std::string> > settingsObserver;
            std::shared_ptr<observer::ListObserver<std::shared_ptr<play::FilesModelItem> > > filesObserver;
            std::shared_ptr<observer::ListObserver<std::shared_ptr<play::FilesModelItem> > > activeObserver;
            std::shared_ptr<observer::ListObserver<int> > layersObserver;
            std::shared_ptr<observer::ValueObserver<size_t> > recentFilesMaxObserver;
            std::shared_ptr<observer::ListObserver<file::Path> > recentFilesObserver;
            std::shared_ptr<observer::ValueObserver<float> > volumeObserver;
            std::shared_ptr<observer::ValueObserver<bool> > muteObserver;
            std::shared_ptr<observer::ValueObserver<double> > syncOffsetObserver;
#if defined(TLRENDER_BMD)
            std::shared_ptr<observer::ValueObserver<BMDDevicesModelData> > bmdDevicesObserver;
#endif // TLRENDER_BMD
        };

        App::App(
            int& argc,
            char** argv,
            const std::shared_ptr<system::Context>& context) :
            QApplication(argc, argv),
            _p(new Private)
        {
            TLRENDER_P();
            const std::string appName = "tlplay-qt";
            const std::string appDocsPath = play::appDocsPath();
            const std::string logFileName = play::logFileName(appName, appDocsPath);
            const std::string settingsFileName = play::settingsName(appName, appDocsPath);
            IApp::_init(
                app::convert(argc, argv),
                context,
                appName,
                "Example Qt playback application.",
                play::getCmdLineArgs(p.options),
                play::getCmdLineOptions(p.options, logFileName, settingsFileName));
            const int exitCode = getExit();
            if (exitCode != 0)
            {
                exit(exitCode);
                return;
            }

            setOrganizationName("tlRender");
            setApplicationName(QString::fromUtf8(appName.c_str()));
            setStyle("Fusion");
            setPalette(qtwidget::darkStyle());
            setStyleSheet(qtwidget::styleSheet());
            qtwidget::initFonts(context);

            _fileLogInit(logFileName);
            _settingsInit(settingsFileName);
            _modelsInit();
            _observersInit();
            _inputFilesInit();
            _windowsInit();
        }

        App::~App()
        {}

        const std::shared_ptr<timeline::TimeUnitsModel>& App::timeUnitsModel() const
        {
            return _p->timeUnitsModel;
        }

        qt::TimeObject* App::timeObject() const
        {
            return _p->timeObject.get();
        }

        const std::shared_ptr<play::Settings>& App::settings() const
        {
            return _p->settings;
        }

        const std::shared_ptr<play::FilesModel>& App::filesModel() const
        {
            return _p->filesModel;
        }

        const QVector<QSharedPointer<qt::TimelinePlayer> >& App::players() const
        {
            return _p->players;
        }

        QVector<QSharedPointer<qt::TimelinePlayer> > App::activePlayers() const
        {
            TLRENDER_P();
            QVector<QSharedPointer<qt::TimelinePlayer> > out;
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

        const std::shared_ptr<ui::RecentFilesModel>& App::recentFilesModel() const
        {
            return _p->recentFilesModel;
        }

        const std::shared_ptr<play::ViewportModel>& App::viewportModel() const
        {
            return _p->viewportModel;
        }

        const std::shared_ptr<play::ColorModel>& App::colorModel() const
        {
            return _p->colorModel;
        }

        const std::shared_ptr<play::AudioModel>& App::audioModel() const
        {
            return _p->audioModel;
        }

        MainWindow* App::mainWindow() const
        {
            return _p->mainWindow.get();
        }

#if defined(TLRENDER_BMD)
        const std::shared_ptr<BMDDevicesModel>& App::bmdDevicesModel() const
        {
            return _p->bmdDevicesModel;
        }

        qt::BMDOutputDevice* App::bmdOutputDevice() const
        {
            return _p->bmdOutputDevice.get();
        }
#endif // TLRENDER_BMD

        void App::open(const QString& fileName, const QString& audioFileName)
        {
            TLRENDER_P();
            file::PathOptions pathOptions;
            pathOptions.maxNumberDigits = p.settings->getValue<size_t>("FileSequence/MaxDigits");
            for (const auto& path :
                timeline::getPaths(file::Path(fileName.toUtf8().data()), pathOptions, _context))
            {
                auto item = std::make_shared<play::FilesModelItem>();
                item->path = path;
                item->audioPath = file::Path(audioFileName.toUtf8().data());
                p.filesModel->add(item);
                p.recentFilesModel->addRecent(path);
            }
        }

        void App::openDialog()
        {
            TLRENDER_P();
            if (auto fileBrowserSystem = _context->getSystem<qtwidget::FileBrowserSystem>())
            {
                fileBrowserSystem->open(
                    p.mainWindow.get(),
                    [this](const file::Path& value)
                    {
                        if (!value.isEmpty())
                        {
                            open(QString::fromUtf8(value.get().c_str()));
                        }
                    });
            }
        }

        void App::openSeparateAudioDialog()
        {
            QScopedPointer<OpenSeparateAudioDialog> dialog(new OpenSeparateAudioDialog(_context));
            if (QDialog::Accepted == dialog->exec())
            {
                open(dialog->videoFileName(), dialog->audioFileName());
            }
        }

        void App::setSecondaryWindow(bool value)
        {
            TLRENDER_P();
            //! \bug macOS does not seem to like having an application with
            //! normal and fullscreen windows.
            QScreen* secondaryScreen = nullptr;
#if !defined(__APPLE__)
            auto screens = this->screens();
            auto mainWindowScreen = p.mainWindow->screen();
            screens.removeOne(mainWindowScreen);
            if (!screens.isEmpty())
            {
                secondaryScreen = screens[0];
            }
#endif // __APPLE__
            if (value)
            {
                p.secondaryWindow.reset(new SecondaryWindow(this));
                if (secondaryScreen)
                {
                    p.secondaryWindow->move(secondaryScreen->availableGeometry().topLeft());
                    p.secondaryWindow->setWindowState(
                        p.secondaryWindow->windowState() ^ Qt::WindowFullScreen);
                }

                connect(
                    p.secondaryWindow.get(),
                    SIGNAL(destroyed(QObject*)),
                    SLOT(_secondaryWindowDestroyedCallback()));

                p.secondaryWindow->show();
            }
            else if (p.secondaryWindow)
            {
                p.secondaryWindow->close();
            }
            Q_EMIT secondaryWindowChanged(value);
        }

        void App::_filesCallback(const std::vector<std::shared_ptr<play::FilesModelItem> >& items)
        {
            TLRENDER_P();

            // Create the new list of players.
            QVector<QSharedPointer<qt::TimelinePlayer> > players(items.size(), nullptr);
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
            QVector<QSharedPointer<qt::TimelinePlayer> > destroy;
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
                        options.ioOptions = _ioOptions();
                        options.pathOptions.maxNumberDigits =
                            p.settings->getValue<size_t>("FileSequence/MaxDigits");
                        auto otioTimeline = items[i]->audioPath.isEmpty() ?
                            timeline::create(items[i]->path, _context, options) :
                            timeline::create(items[i]->path, items[i]->audioPath, _context, options);
                        if (0)
                        {
                            timeline::toMemoryReferences(
                                otioTimeline,
                                items[i]->path.getDirectory(),
                                timeline::ToMemoryReference::Shared,
                                options.pathOptions);
                        }
                        auto timeline = timeline::Timeline::create(otioTimeline, _context, options);
                        const otime::TimeRange& timeRange = timeline->getTimeRange();

                        timeline::PlayerOptions playerOptions;
                        playerOptions.cache.readAhead = time::invalidTime;
                        playerOptions.cache.readBehind = time::invalidTime;
                        playerOptions.timerMode =
                            p.settings->getValue<timeline::TimerMode>("Performance/TimerMode");
                        playerOptions.audioBufferFrameCount =
                            p.settings->getValue<size_t>("Performance/AudioBufferFrameCount");
                        auto player = timeline::Player::create(timeline, _context, playerOptions);
                        players[i].reset(new qt::TimelinePlayer(player, _context, this));

                        for (const auto& video : player->getIOInfo().video)
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

            auto activePlayers = this->activePlayers();
            if (!activePlayers.empty() && activePlayers[0])
            {
                activePlayers[0]->setPlayback(timeline::Playback::Stop);
            }

            p.activeFiles = items;
            activePlayers = this->activePlayers();
            QSharedPointer<qt::TimelinePlayer> first;
            if (!activePlayers.empty())
            {
                first = activePlayers[0];
                if (first)
                {
                    first->player()->setExternalTime(nullptr);
                }
            }
            for (size_t i = 1; i < activePlayers.size(); ++i)
            {
                if (auto player = activePlayers[i])
                {
                    player->player()->setExternalTime(
                        (first && first != player) ?
                        first->player() :
                        nullptr);
                }
            }
#if defined(TLRENDER_BMD)
            p.bmdOutputDevice->setTimelinePlayers(activePlayers);
#endif // TLRENDER_BMD

            _cacheUpdate();
            _audioUpdate();

            Q_EMIT activePlayersChanged(activePlayers);
        }

        void App::_mainWindowDestroyedCallback()
        {
            TLRENDER_P();
            p.mainWindow.take();
            if (p.secondaryWindow)
            {
                p.secondaryWindow->close();
            }
        }

        void App::_secondaryWindowDestroyedCallback()
        {
            TLRENDER_P();
            p.secondaryWindow.take();
            Q_EMIT secondaryWindowChanged(false);
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
            if (!p.options.settingsFileName.empty())
            {
                p.settingsFileName = p.options.settingsFileName;
            }
            else
            {
                p.settingsFileName = settingsFileName;
            }
            p.settings = play::Settings::create(
                p.settingsFileName,
                p.options.resetSettings,
                _context);

            p.settings->setDefaultValue("Files/RecentMax", 10);

            p.settings->setDefaultValue("Cache/Size", 1);
            p.settings->setDefaultValue("Cache/ReadAhead", 2.0);
            p.settings->setDefaultValue("Cache/ReadBehind", 0.5);

            p.settings->setDefaultValue("FileSequence/Audio",
                timeline::FileSequenceAudio::BaseName);
            p.settings->setDefaultValue("FileSequence/AudioFileName", std::string());
            p.settings->setDefaultValue("FileSequence/AudioDirectory", std::string());
            p.settings->setDefaultValue("FileSequence/MaxDigits", 9);

            p.settings->setDefaultValue("SequenceIO/ThreadCount", 16);

#if defined(TLRENDER_BMD)
            BMDDevicesModelData bmdDevicesModelData;
            p.settings->setDefaultValue("BMD/DeviceIndex", bmdDevicesModelData.deviceIndex);
            p.settings->setDefaultValue("BMD/DisplayModeIndex", bmdDevicesModelData.displayModeIndex);
            p.settings->setDefaultValue("BMD/PixelTypeIndex", bmdDevicesModelData.pixelTypeIndex);
            p.settings->setDefaultValue("BMD/DeviceEnabled", bmdDevicesModelData.deviceEnabled);
            const auto i = bmdDevicesModelData.boolOptions.find(device::Option::_444SDIVideoOutput);
            p.settings->setDefaultValue("BMD/444SDIVideoOutput", i != bmdDevicesModelData.boolOptions.end() ? i->second : false);
            p.settings->setDefaultValue("BMD/HDRMode", bmdDevicesModelData.hdrMode);
            p.settings->setDefaultValue("BMD/HDRData", bmdDevicesModelData.hdrData);
#endif // TLRENDER_BMD

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

            p.settings->setDefaultValue("Performance/TimerMode",
                timeline::PlayerOptions().timerMode);
            p.settings->setDefaultValue("Performance/AudioBufferFrameCount",
                timeline::PlayerOptions().audioBufferFrameCount);
            p.settings->setDefaultValue("Performance/VideoRequestCount", 16);
            p.settings->setDefaultValue("Performance/AudioRequestCount", 16);

            p.settings->setDefaultValue("Misc/ToolTipsEnabled", true);
        }

        void App::_modelsInit()
        {
            TLRENDER_P();

            p.contextObject.reset(new qt::ContextObject(_context));

            p.timeUnitsModel = timeline::TimeUnitsModel::create(_context);

            p.timeObject.reset(new qt::TimeObject(p.timeUnitsModel));

            p.filesModel = play::FilesModel::create(_context);

            p.recentFilesModel = ui::RecentFilesModel::create(_context);

            p.viewportModel = play::ViewportModel::create(p.settings, _context);

            p.colorModel = play::ColorModel::create(_context);
            p.colorModel->setOCIOOptions(p.options.ocioOptions);
            p.colorModel->setLUTOptions(p.options.lutOptions);

            if (auto audioSystem = _context->getSystem<audio::System>())
            {
                p.audioInfo = audioSystem->getDefaultOutputInfo();
            }
            p.audioModel = play::AudioModel::create(p.settings, _context);

#if defined(TLRENDER_BMD)
            p.bmdOutputDevice.reset(new qt::BMDOutputDevice(_context));
            if (0)
            {
                QImage* bmdOverlayImage = new QImage(1920, 1080, QImage::Format_RGBA8888);
                bmdOverlayImage->fill(QColor(0, 0, 255, 63));
                p.bmdOutputDevice->setOverlay(bmdOverlayImage);
            }
            connect(
                p.bmdOutputDevice.get(),
                &qt::BMDOutputDevice::deviceActiveChanged,
                [this](bool value)
                {
                    _p->bmdDeviceActive = value;
                    _audioUpdate();
                });
            /*connect(
                p.bmdOutputDevice.get(),
                &qt::BMDOutputDevice::sizeChanged,
                [this](const math::Size2i& value)
                {
                    std::cout << "output device size: " << value << std::endl;
                });
            connect(
                p.bmdOutputDevice.get(),
                &qt::BMDOutputDevice::frameRateChanged,
                [this](const otime::RationalTime& value)
                {
                    std::cout << "output device frame rate: " << value << std::endl;
                });*/
            p.bmdDevicesModel = BMDDevicesModel::create(_context); p.bmdDevicesModel->setDeviceIndex(
                p.settings->getValue<int>("BMD/DeviceIndex"));
            p.bmdDevicesModel->setDisplayModeIndex(
                p.settings->getValue<int>("BMD/DisplayModeIndex"));
            p.bmdDevicesModel->setPixelTypeIndex(
                p.settings->getValue<int>("BMD/PixelTypeIndex"));
            p.bmdDevicesModel->setDeviceEnabled(
                p.settings->getValue<bool>("BMD/DeviceEnabled"));
            device::BoolOptions deviceBoolOptions;
            deviceBoolOptions[device::Option::_444SDIVideoOutput] =
                p.settings->getValue<bool>("BMD/444SDIVideoOutput");
            p.bmdDevicesModel->setBoolOptions(deviceBoolOptions);
            p.bmdDevicesModel->setHDRMode(static_cast<device::HDRMode>(
                p.settings->getValue<int>("BMD/HDRMode")));
            std::string s = p.settings->getValue<std::string>("BMD/HDRData");
            if (!s.empty())
            {
                auto json = nlohmann::json::parse(s);
                image::HDRData hdrData;
                try
                {
                    from_json(json, hdrData);
                }
                catch (const std::exception&)
                {
                }
                p.bmdDevicesModel->setHDRData(hdrData);
            }
#endif // TLRENDER_BMD
        }

        void App::_observersInit()
        {
            TLRENDER_P();

            p.settingsObserver = observer::ValueObserver<std::string>::create(
                p.settings->observeValues(),
                [this](const std::string& name)
                {
                    _settingsUpdate(name);
                });

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
                        if (_p->players[i])
                        {
                            io::Options ioOptions;
                            ioOptions["Layer"] = string::Format("{0}").arg(value[i]);
                            _p->players[i]->setIOOptions(ioOptions);
                        }
                    }
                });

            p.recentFilesMaxObserver = observer::ValueObserver<size_t>::create(
                p.recentFilesModel->observeRecentMax(),
                [this](size_t value)
                {
                    _p->settings->setValue("Files/RecentMax", value);
                });
            p.recentFilesObserver = observer::ListObserver<file::Path>::create(
                p.recentFilesModel->observeRecent(),
                [this](const std::vector<file::Path>& value)
                {
                    std::vector<std::string> fileNames;
                    for (const auto& i : value)
                    {
                        fileNames.push_back(i.get());
                    }
                    _p->settings->setValue("Files/Recent", fileNames);
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

#if defined(TLRENDER_BMD)
            p.bmdDevicesObserver = observer::ValueObserver<BMDDevicesModelData>::create(
                p.bmdDevicesModel->observeData(),
                [this](const BMDDevicesModelData& value)
                {
                    TLRENDER_P();
                    const device::PixelType pixelType = value.pixelTypeIndex >= 0 &&
                        value.pixelTypeIndex < value.pixelTypes.size() ?
                        value.pixelTypes[value.pixelTypeIndex] :
                        device::PixelType::None;
                    p.bmdOutputDevice->setDevice(
                        value.deviceIndex - 1,
                        value.displayModeIndex - 1,
                        pixelType);
                    p.bmdOutputDevice->setDeviceEnabled(value.deviceEnabled);
                    p.bmdOutputDevice->setBoolOptions(value.boolOptions);
                    p.bmdOutputDevice->setHDR(value.hdrMode, value.hdrData);

                    p.settings->setValue("BMD/DeviceIndex", value.deviceIndex);
                    p.settings->setValue("BMD/DisplayModeIndex", value.displayModeIndex);
                    p.settings->setValue("BMD/PixelTypeIndex", value.pixelTypeIndex);
                    p.settings->setValue("BMD/DeviceEnabled", value.deviceEnabled);
                    const auto i = value.boolOptions.find(device::Option::_444SDIVideoOutput);
                    p.settings->setValue("BMD/444SDIVideoOutput", i != value.boolOptions.end() ? i->second : false);
                    p.settings->setValue("BMD/HDRMode", value.hdrMode);
                    p.settings->setValue("BMD/HDRData", value.hdrData);
                });
#endif // TLRENDER_BMD
        }

        void App::_inputFilesInit()
        {
            TLRENDER_P();
            if (!p.options.fileName.empty())
            {
                if (!p.options.compareFileName.empty())
                {
                    open(QString::fromUtf8(p.options.compareFileName.c_str()));
                    p.filesModel->setCompareOptions(p.options.compareOptions);
                    p.filesModel->setB(0, true);
                }

                open(
                    QString::fromUtf8(p.options.fileName.c_str()),
                    QString::fromUtf8(p.options.audioFileName.c_str()));

                if (!p.players.empty() && p.players[0])
                {
                    if (p.options.speed > 0.0)
                    {
                        p.players[0]->setSpeed(p.options.speed);
                    }
                    if (time::isValid(p.options.inOutRange))
                    {
                        p.players[0]->setInOutRange(p.options.inOutRange);
                        p.players[0]->seek(p.options.inOutRange.start_time());
                    }
                    if (time::isValid(p.options.seek))
                    {
                        p.players[0]->seek(p.options.seek);
                    }
                    p.players[0]->setLoop(p.options.loop);
                    p.players[0]->setPlayback(p.options.playback);
                }
            }
        }

        void App::_windowsInit()
        {
            TLRENDER_P();

            p.mainWindow.reset(new MainWindow(this));
            const math::Size2i windowSize = p.settings->getValue<math::Size2i>("MainWindow/Size");
            p.mainWindow->resize(windowSize.w, windowSize.h);
            p.mainWindow->show();

            connect(
                p.mainWindow.get(),
                SIGNAL(destroyed(QObject*)),
                SLOT(_mainWindowDestroyedCallback()));
        }

        io::Options App::_ioOptions() const
        {
            TLRENDER_P();
            io::Options out;

            out["SequenceIO/ThreadCount"] = string::Format("{0}").
                arg(p.settings->getValue<int>("SequenceIO/ThreadCount"));

#if defined(TLRENDER_FFMPEG)
            out["FFmpeg/YUVToRGBConversion"] = string::Format("{0}").
                arg(p.settings->getValue<bool>("FFmpeg/YUVToRGBConversion"));
            out["FFmpeg/AudioChannelCount"] = string::Format("{0}").
                arg(p.audioInfo.channelCount);
            out["FFmpeg/AudioDataType"] = string::Format("{0}").
                arg(p.audioInfo.dataType);
            out["FFmpeg/AudioSampleRate"] = string::Format("{0}").
                arg(p.audioInfo.sampleRate);
            out["FFmpeg/ThreadCount"] = string::Format("{0}").
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

        otime::RationalTime App::_cacheReadAhead() const
        {
            TLRENDER_P();
            const size_t activeCount = p.filesModel->observeActive()->getSize();
            const double readAhead = p.settings->getValue<double>("Cache/ReadAhead");
            return otime::RationalTime(
                activeCount > 0 ? (readAhead / static_cast<double>(activeCount)) : 0.0,
                1.0);
        }

        otime::RationalTime App::_cacheReadBehind() const
        {
            TLRENDER_P();
            const size_t activeCount = p.filesModel->observeActive()->getSize();
            const double readBehind = p.settings->getValue<double>("Cache/ReadBehind");
            return otime::RationalTime(
                activeCount > 0 ? (readBehind / static_cast<double>(activeCount)) : 0.0,
                1.0);
        }

        void App::_settingsUpdate(const std::string& name)
        {
            TLRENDER_P();
            const auto split = string::split(name, '/');
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
                    const auto ioOptions = _ioOptions();
                    for (const auto& player : p.players)
                    {
                        if (player)
                        {
                            player->setIOOptions(ioOptions);
                        }
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
            if ("FileBrowser/NativeFileDialog" == name || name.empty())
            {
                auto fileBrowserSystem = _context->getSystem<qtwidget::FileBrowserSystem>();
                fileBrowserSystem->setNativeFileDialog(
                    p.settings->getValue<bool>("FileBrowser/NativeFileDialog"));
            }
            if ("Files/RecentMax" == name || name.empty())
            {
                p.recentFilesModel->setRecentMax(
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
                p.recentFilesModel->setRecent(recentPaths);
            }
            if ("Misc/ToolTipsEnabled" == name || name.empty())
            {
                if (p.settings->getValue<bool>("Misc/ToolTipsEnabled"))
                {
                    removeEventFilter(p.toolTipsFilter.get());
                    p.toolTipsFilter.reset();
                }
                else
                {
                    p.toolTipsFilter.reset(new qt::ToolTipsFilter(this));
                    installEventFilter(p.toolTipsFilter.get());
                }
            }
        }

        void App::_cacheUpdate()
        {
            TLRENDER_P();

            const auto activePlayers = this->activePlayers();

            // Update the I/O cache.
            auto ioSystem = _context->getSystem<io::System>();
            ioSystem->getCache()->setMax(
                p.settings->getValue<size_t>("Cache/Size") * memory::gigabyte);

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
            cacheOptions.readAhead = _cacheReadAhead();
            cacheOptions.readBehind = _cacheReadBehind();
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
            const bool mute = p.audioModel->isMuted();
            for (auto player : p.players)
            {
                if (player)
                {
                    player->setVolume(volume);
                    player->setMute(mute || p.bmdDeviceActive);
                }
            }
#if defined(TLRENDER_BMD)
            if (p.bmdOutputDevice)
            {
                p.bmdOutputDevice->setVolume(volume);
                p.bmdOutputDevice->setMute(mute);
                const auto activePlayers = this->activePlayers();
                p.bmdOutputDevice->setAudioOffset(
                    (!activePlayers.empty() && activePlayers[0]) ?
                    activePlayers[0]->audioOffset() :
                    0.0);
            }
#endif // TLRENDER_BMD
        }
    }
}
