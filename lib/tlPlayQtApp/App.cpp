// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2023 Darby Johnston
// All rights reserved.

#include <tlPlayQtApp/App.h>

#include <tlPlayQtApp/DevicesModel.h>
#include <tlPlayQtApp/MainWindow.h>
#include <tlPlayQtApp/OpenSeparateAudioDialog.h>
#include <tlPlayQtApp/SecondaryWindow.h>

#include <tlPlay/Settings.h>

#include <tlQtWidget/Init.h>
#include <tlQtWidget/FileBrowserSystem.h>
#include <tlQtWidget/Style.h>
#include <tlQtWidget/TimelineViewport.h>
#include <tlQtWidget/Util.h>

#include <tlQt/ContextObject.h>
#include <tlQt/MetaTypes.h>
#include <tlQt/OutputDevice.h>
#include <tlQt/TimeObject.h>
#include <tlQt/TimelinePlayer.h>
#include <tlQt/ToolTipsFilter.h>

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
            std::vector<std::shared_ptr<timeline::Timeline> > timelines;
            QSharedPointer<qt::TimelinePlayer> player;
            std::shared_ptr<ui::RecentFilesModel> recentFilesModel;
            std::shared_ptr<play::ViewportModel> viewportModel;
            std::shared_ptr<play::ColorModel> colorModel;
            QScopedPointer<qt::OutputDevice> outputDevice;
            bool deviceActive = false;
            std::shared_ptr<DevicesModel> devicesModel;
            audio::Info audioInfo;
            std::shared_ptr<play::AudioModel> audioModel;
            QScopedPointer<qt::ToolTipsFilter> toolTipsFilter;

            QScopedPointer<MainWindow> mainWindow;
            QScopedPointer<SecondaryWindow> secondaryWindow;

            std::shared_ptr<observer::ValueObserver<std::string> > settingsObserver;
            std::shared_ptr<observer::ListObserver<std::shared_ptr<play::FilesModelItem> > > filesObserver;
            std::shared_ptr<observer::ListObserver<std::shared_ptr<play::FilesModelItem> > > activeObserver;
            std::shared_ptr<observer::ListObserver<int> > layersObserver;
            std::shared_ptr<observer::ValueObserver<size_t> > recentFilesMaxObserver;
            std::shared_ptr<observer::ListObserver<file::Path> > recentFilesObserver;
            std::shared_ptr<observer::ValueObserver<DevicesModelData> > devicesObserver;
            std::shared_ptr<observer::ValueObserver<float> > volumeObserver;
            std::shared_ptr<observer::ValueObserver<bool> > muteObserver;
            std::shared_ptr<observer::ValueObserver<double> > syncOffsetObserver;
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

        const QSharedPointer<qt::TimelinePlayer>& App::player() const
        {
            return _p->player;
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

        qt::OutputDevice* App::outputDevice() const
        {
            return _p->outputDevice.get();
        }

        const std::shared_ptr<DevicesModel>& App::devicesModel() const
        {
            return _p->devicesModel;
        }

        const std::shared_ptr<play::AudioModel>& App::audioModel() const
        {
            return _p->audioModel;
        }

        MainWindow* App::mainWindow() const
        {
            return _p->mainWindow.get();
        }

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
                    SLOT(_secondaryWindowDestroyed()));

                p.secondaryWindow->show();
            }
            else if (p.secondaryWindow)
            {
                p.secondaryWindow->close();
            }
            Q_EMIT secondaryWindowChanged(value);
        }

        void App::_filesUpdate(const std::vector<std::shared_ptr<play::FilesModelItem> >& items)
        {
            TLRENDER_P();

            // Keep existing timelines.
            std::vector<std::shared_ptr<timeline::Timeline> > timelines(items.size());
            for (size_t i = 0; i < items.size(); ++i)
            {
                const auto j = std::find(p.files.begin(), p.files.end(), items[i]);
                if (j != p.files.end())
                {
                    const size_t k = j - p.files.begin();
                    timelines[i] = p.timelines[k];
                }
            }

            // Find timelines to destroy.
            std::vector<std::shared_ptr<timeline::Timeline> > destroy;
            for (size_t i = 0; i < p.files.size(); ++i)
            {
                const auto j = std::find(items.begin(), items.end(), p.files[i]);
                if (j == items.end())
                {
                    destroy.push_back(p.timelines[i]);
                }
            }

            // Create new timelines.
            auto audioSystem = _context->getSystem<audio::System>();
            for (size_t i = 0; i < timelines.size(); ++i)
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
                        options.ioOptions = _ioOptions();
                        options.pathOptions.maxNumberDigits =
                            p.settings->getValue<size_t>("FileSequence/MaxDigits");
                        auto otioTimeline = items[i]->audioPath.isEmpty() ?
                            timeline::create(items[i]->path, _context, options) :
                            timeline::create(items[i]->path, items[i]->audioPath, _context, options);
                        timelines[i] = timeline::Timeline::create(otioTimeline, _context, options);

                        for (const auto& video : timelines[i]->getIOInfo().video)
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
            p.timelines = timelines;
        }

        void App::_activeUpdate(const std::vector<std::shared_ptr<play::FilesModelItem> >& items)
        {
            TLRENDER_P();

            if (p.player)
            {
                p.player->setPlayback(timeline::Playback::Stop);
            }

            p.activeFiles = items;

            auto activeTimelines = _activeTimelines();
            if (!activeTimelines.empty())
            {
                if (!p.player || (p.player && activeTimelines[0] != p.player->timeline()))
                {
                    timeline::PlayerOptions playerOptions;
                    playerOptions.cache.readAhead = time::invalidTime;
                    playerOptions.cache.readBehind = time::invalidTime;
                    playerOptions.timerMode =
                        p.settings->getValue<timeline::TimerMode>("Performance/TimerMode");
                    playerOptions.audioBufferFrameCount =
                        p.settings->getValue<size_t>("Performance/AudioBufferFrameCount");
                    auto player = timeline::Player::create(activeTimelines[0], _context, playerOptions);
                    p.player.reset(new qt::TimelinePlayer(player, _context, this));
                    Q_EMIT playerChanged(p.player);
                }
            }
            else
            {
                Q_EMIT playerChanged(nullptr);
            }

            if (p.player)
            {
                if (!activeTimelines.empty())
                {
                    std::vector<std::shared_ptr<timeline::Timeline> > compare;
                    for (size_t i = 1; i < activeTimelines.size(); ++i)
                    {
                        if (activeTimelines[i])
                        {
                            compare.push_back(activeTimelines[i]);
                        }
                    }
                    p.player->setCompare(compare);
                }
            }

            p.outputDevice->setTimelinePlayer(p.player);

            _cacheUpdate();
            _audioUpdate();
        }

        void App::_mainWindowDestroyed()
        {
            TLRENDER_P();
            p.mainWindow.take();
            if (p.secondaryWindow)
            {
                p.secondaryWindow->close();
            }
        }

        void App::_secondaryWindowDestroyed()
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

            DevicesModelData devicesModelData;
            p.settings->setDefaultValue("Devices/DeviceIndex", devicesModelData.deviceIndex);
            p.settings->setDefaultValue("Devices/DisplayModeIndex", devicesModelData.displayModeIndex);
            p.settings->setDefaultValue("Devices/PixelTypeIndex", devicesModelData.pixelTypeIndex);
            p.settings->setDefaultValue("Devices/DeviceEnabled", devicesModelData.deviceEnabled);
            p.settings->setDefaultValue("Devices/HDRMode", devicesModelData.hdrMode);
            p.settings->setDefaultValue("Devices/HDRData", devicesModelData.hdrData);

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

            p.outputDevice.reset(new qt::OutputDevice(_context));
            connect(
                p.outputDevice.get(),
                &qt::OutputDevice::deviceActiveChanged,
                [this](bool value)
                {
                    _p->deviceActive = value;
                    _audioUpdate();
                });
            /*connect(
                p.outputDevice.get(),
                &qt::OutputDevice::sizeChanged,
                [this](const math::Size2i& value)
                {
                    std::cout << "output device size: " << value << std::endl;
                });
            connect(
                p.outputDevice,
                &qt::OutputDevice::frameRateChanged,
                [this](const otime::RationalTime& value)
                {
                    std::cout << "output device frame rate: " << value << std::endl;
                });*/
            p.devicesModel = DevicesModel::create(_context);

            if (auto audioSystem = _context->getSystem<audio::System>())
            {
                p.audioInfo = audioSystem->getDefaultOutputInfo();
            }
            p.audioModel = play::AudioModel::create(p.settings, _context);
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
                    _filesUpdate(value);
                });
            p.activeObserver = observer::ListObserver<std::shared_ptr<play::FilesModelItem> >::create(
                p.filesModel->observeActive(),
                [this](const std::vector<std::shared_ptr<play::FilesModelItem> >& value)
                {
                    _activeUpdate(value);
                });
            p.layersObserver = observer::ListObserver<int>::create(
                p.filesModel->observeLayers(),
                [this](const std::vector<int>& value)
                {
                    _layersUpdate(value);
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

            p.devicesObserver = observer::ValueObserver<DevicesModelData>::create(
                p.devicesModel->observeData(),
                [this](const DevicesModelData& value)
                {
                    TLRENDER_P();
                    const device::PixelType pixelType = value.pixelTypeIndex >= 0 &&
                        value.pixelTypeIndex < value.pixelTypes.size() ?
                        value.pixelTypes[value.pixelTypeIndex] :
                        device::PixelType::None;
                    p.outputDevice->setDevice(
                        value.deviceIndex - 1,
                        value.displayModeIndex - 1,
                        pixelType);
                    p.outputDevice->setDeviceEnabled(value.deviceEnabled);
                    p.outputDevice->setHDR(value.hdrMode, value.hdrData);

                    p.settings->setValue("Devices/DeviceIndex", value.deviceIndex);
                    p.settings->setValue("Devices/DisplayModeIndex", value.displayModeIndex);
                    p.settings->setValue("Devices/PixelTypeIndex", value.pixelTypeIndex);
                    p.settings->setValue("Devices/DeviceEnabled", value.deviceEnabled);
                    p.settings->setValue("Devices/HDRMode", value.hdrMode);
                    p.settings->setValue("Devices/HDRData", value.hdrData);
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

                if (p.player)
                {
                    if (p.options.speed > 0.0)
                    {
                        p.player->setSpeed(p.options.speed);
                    }
                    if (time::isValid(p.options.inOutRange))
                    {
                        p.player->setInOutRange(p.options.inOutRange);
                        p.player->seek(p.options.inOutRange.start_time());
                    }
                    if (time::isValid(p.options.seek))
                    {
                        p.player->seek(p.options.seek);
                    }
                    p.player->setLoop(p.options.loop);
                    p.player->setPlayback(p.options.playback);
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
                SLOT(_mainWindowDestroyed()));
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

        std::vector<std::shared_ptr<timeline::Timeline> > App::_activeTimelines() const
        {
            TLRENDER_P();
            std::vector<std::shared_ptr<timeline::Timeline> > out;
            for (size_t i = 0; i < p.activeFiles.size(); ++i)
            {
                const auto j = std::find(
                    p.files.begin(),
                    p.files.end(),
                    p.activeFiles[i]);
                if (j != p.files.end())
                {
                    const auto k = j - p.files.begin();
                    out.push_back(p.timelines[k]);
                }
            }
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

        void App::_layersUpdate(const std::vector<int>& value)
        {
            TLRENDER_P();
            if (p.player)
            {
                if (!value.empty())
                {
                    io::Options ioOptions = _ioOptions();
                    ioOptions["Layer"] = string::Format("{0}").arg(value[0]);
                    p.player->setIOOptions(ioOptions);
                }
                std::vector<int> compareLayers;
                for (size_t i = 1; i < value.size() && i < p.timelines.size(); ++i)
                {
                    compareLayers.push_back(value[i]);
                }
                p.player->setCompareLayers(compareLayers);
            }
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
                    if (p.player)
                    {
                        const io::Options ioOptions = _ioOptions();
                        p.player->setIOOptions(ioOptions);
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

            // Update the I/O cache.
            auto ioSystem = _context->getSystem<io::System>();
            ioSystem->getCache()->setMax(
                p.settings->getValue<size_t>("Cache/Size") * memory::gigabyte);

            // Update the player.
            if (p.player)
            {
                timeline::PlayerCacheOptions cacheOptions;
                cacheOptions.readAhead = _cacheReadAhead();
                cacheOptions.readBehind = _cacheReadBehind();
                p.player->setCacheOptions(cacheOptions);
            }
        }

        void App::_audioUpdate()
        {
            TLRENDER_P();
            const float volume = p.audioModel->getVolume();
            const bool mute = p.audioModel->isMuted();
            if (p.player)
            {
                p.player->setVolume(volume);
                p.player->setMute(mute || p.deviceActive);
            }
            if (p.outputDevice)
            {
                p.outputDevice->setVolume(volume);
                p.outputDevice->setMute(mute);
                p.outputDevice->setAudioOffset(
                    p.player ?
                    p.player->audioOffset() :
                    0.0);
            }
        }
    }
}
