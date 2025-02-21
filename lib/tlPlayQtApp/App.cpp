// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2025 Darby Johnston
// All rights reserved.

#include <tlPlayQtApp/App.h>

#include <tlPlayQtApp/MainWindow.h>
#include <tlPlayQtApp/OpenSeparateAudioDialog.h>
#include <tlPlayQtApp/SecondaryWindow.h>

#include <tlQtWidget/Init.h>
#include <tlQtWidget/FileBrowserSystem.h>
#include <tlQtWidget/Style.h>
#include <tlQtWidget/Util.h>

#include <tlQt/ContextObject.h>
#include <tlQt/MetaTypes.h>
#include <tlQt/TimeObject.h>
#include <tlQt/TimelinePlayer.h>
#include <tlQt/ToolTipsFilter.h>

#include <tlPlay/App.h>
#include <tlPlay/AudioModel.h>
#include <tlPlay/ColorModel.h>
#include <tlPlay/FilesModel.h>
#include <tlPlay/RecentFilesModel.h>
#include <tlPlay/RenderModel.h>
#include <tlPlay/SettingsModel.h>
#include <tlPlay/Viewport.h>
#include <tlPlay/ViewportModel.h>
#include <tlPlay/Util.h>
#if defined(TLRENDER_BMD)
#include <tlPlay/BMDDevicesModel.h>
#endif // TLRENDER_BMD

#if defined(TLRENDER_BMD)
#include <tlDevice/BMDOutputDevice.h>
#endif // TLRENDER_BMD

#include <tlTimelineUI/ThumbnailSystem.h>

#include <tlTimeline/Util.h>

#include <tlIO/System.h>
#if defined(TLRENDER_USD)
#include <tlIO/USD.h>
#endif // TLRENDER_USD

#include <tlCore/AudioSystem.h>
#include <tlCore/FileLogSystem.h>
#include <tlCore/Time.h>

#include <dtk/ui/Settings.h>
#include <dtk/core/Format.h>
#include <dtk/core/Math.h>

#include <QPointer>
#include <QScreen>
#include <QTimer>

namespace tl
{
    namespace play_qt
    {
        namespace
        {
            const size_t timeout = 5;
        }

        struct App::Private
        {
            play::Options options;
            QScopedPointer<qt::ContextObject> contextObject;
            std::shared_ptr<file::FileLogSystem> fileLogSystem;
            std::shared_ptr<dtk::Settings> settings;
            std::shared_ptr<play::SettingsModel> settingsModel;
            std::shared_ptr<timeline::TimeUnitsModel> timeUnitsModel;
            QScopedPointer<qt::TimeObject> timeObject;
            std::shared_ptr<play::FilesModel> filesModel;
            std::vector<std::shared_ptr<play::FilesModelItem> > files;
            std::vector<std::shared_ptr<play::FilesModelItem> > activeFiles;
            std::shared_ptr<play::RecentFilesModel> recentFilesModel;
            std::vector<std::shared_ptr<timeline::Timeline> > timelines;
            QSharedPointer<qt::TimelinePlayer> player;
            std::shared_ptr<play::ColorModel> colorModel;
            std::shared_ptr<play::RenderModel> renderModel;
            std::shared_ptr<play::ViewportModel> viewportModel;
            std::shared_ptr<play::AudioModel> audioModel;
            QScopedPointer<qt::ToolTipsFilter> toolTipsFilter;

            QScopedPointer<MainWindow> mainWindow;
            QScopedPointer<SecondaryWindow> secondaryWindow;

            bool bmdDeviceActive = false;
#if defined(TLRENDER_BMD)
            std::shared_ptr<bmd::DevicesModel> bmdDevicesModel;
            std::shared_ptr<bmd::OutputDevice> bmdOutputDevice;
            dtk::VideoLevels bmdOutputVideoLevels = dtk::VideoLevels::First;
#endif // TLRENDER_BMD
            std::unique_ptr<QTimer> timer;

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
#if defined(TLRENDER_BMD)
            std::shared_ptr<dtk::ValueObserver<bmd::DevicesModelData> > bmdDevicesObserver;
            std::shared_ptr<dtk::ValueObserver<bool> > bmdActiveObserver;
            std::shared_ptr<dtk::ValueObserver<dtk::Size2I> > bmdSizeObserver;
            std::shared_ptr<dtk::ValueObserver<bmd::FrameRate> > bmdFrameRateObserver;
            std::shared_ptr<dtk::ValueObserver<timeline::CompareOptions> > compareOptionsObserver;
            std::shared_ptr<dtk::ValueObserver<timeline::OCIOOptions> > ocioOptionsObserver;
            std::shared_ptr<dtk::ValueObserver<timeline::LUTOptions> > lutOptionsObserver;
            std::shared_ptr<dtk::ValueObserver<dtk::ImageOptions> > imageOptionsObserver;
            std::shared_ptr<dtk::ValueObserver<timeline::DisplayOptions> > displayOptionsObserver;
            std::shared_ptr<dtk::ValueObserver<timeline::BackgroundOptions> > backgroundOptionsObserver;
#endif // TLRENDER_BMD
        };

        App::App(
            const std::shared_ptr<dtk::Context>& context,
            int& argc,
            char** argv) :
            QApplication(argc, argv),
            _p(new Private)
        {
            DTK_P();
            const std::string appName = "tlplay-qt";
            const std::filesystem::path appDocsPath = play::appDocsPath();
            p.options.logFile = play::logFileName(appName, appDocsPath).u8string();
            p.options.settingsFile = play::settingsName(appName, appDocsPath).u8string();
            auto args = dtk::convert(argc, argv);
            IApp::_init(
                context,
                args,
                appName,
                "Example Qt playback application.",
                play::getCmdLineArgs(p.options),
                play::getCmdLineOptions(p.options));
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

            p.fileLogSystem = file::FileLogSystem::create(
                _context,
                std::filesystem::u8path(p.options.logFile));
            p.settings = dtk::Settings::create(
                _context,
                std::filesystem::u8path(p.options.settingsFile),
                p.options.resetSettings);
            _modelsInit();
            _devicesInit();
            _observersInit();
            _inputFilesInit();
            _windowsInit();

            p.timer.reset(new QTimer);
            p.timer->setTimerType(Qt::PreciseTimer);
            connect(p.timer.get(), &QTimer::timeout, this, &App::_timerUpdate);
            p.timer->start(timeout);
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

        const std::shared_ptr<play::SettingsModel>& App::settingsModel() const
        {
            return _p->settingsModel;
        }

        const std::shared_ptr<play::FilesModel>& App::filesModel() const
        {
            return _p->filesModel;
        }

        const std::shared_ptr<play::RecentFilesModel>& App::recentFilesModel() const
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
                if (p.player)
                {
                    activeFiles.front()->currentTime = p.player->currentTime();
                }
            }

            auto thumbnailSytem = _context->getSystem<timelineui::ThumbnailSystem>();
            thumbnailSytem->getCache()->clear();

            auto ioSystem = _context->getSystem<io::System>();
            ioSystem->getCache()->clear();

            _filesUpdate(files);
            _activeUpdate(activeFiles);
        }

        const QSharedPointer<qt::TimelinePlayer>& App::player() const
        {
            return _p->player;
        }

        const std::shared_ptr<play::ColorModel>& App::colorModel() const
        {
            return _p->colorModel;
        }

        const std::shared_ptr<play::RenderModel>& App::renderModel() const
        {
            return _p->renderModel;
        }

        const std::shared_ptr<play::ViewportModel>& App::viewportModel() const
        {
            return _p->viewportModel;
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
        const std::shared_ptr<bmd::DevicesModel>& App::bmdDevicesModel() const
        {
            return _p->bmdDevicesModel;
        }

        const std::shared_ptr<bmd::OutputDevice>& App::bmdOutputDevice() const
        {
            return _p->bmdOutputDevice;
        }
#endif // TLRENDER_BMD

        void App::open(const QString& fileName, const QString& audioFileName)
        {
            DTK_P();
            file::PathOptions pathOptions;
            pathOptions.maxNumberDigits = p.settingsModel->getFileSequence().maxDigits;
            for (const auto& path :
                timeline::getPaths(_context, file::Path(fileName.toUtf8().data()), pathOptions))
            {
                auto item = std::make_shared<play::FilesModelItem>();
                item->path = path;
                item->audioPath = file::Path(audioFileName.toUtf8().data());
                p.filesModel->add(item);
                p.recentFilesModel->addRecent(path.get());
            }
        }

        void App::openDialog()
        {
            DTK_P();
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
            DTK_P();
            if (value)
            {
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

                p.secondaryWindow.reset(new SecondaryWindow(this));
                if (secondaryScreen)
                {
                    p.secondaryWindow->move(secondaryScreen->availableGeometry().topLeft());
                    p.secondaryWindow->setWindowState(
                        p.secondaryWindow->windowState() ^ Qt::WindowFullScreen);
                }

                connect(
                    p.secondaryWindow.get(),
                    &QObject::destroyed,
                    [this](QObject*)
                    {
                        _p->secondaryWindow.take();
                        Q_EMIT secondaryWindowChanged(false);
                    });

                p.secondaryWindow->show();
            }
            else if (p.secondaryWindow)
            {
                p.secondaryWindow->close();
            }
            Q_EMIT secondaryWindowChanged(value);
        }

        void App::_modelsInit()
        {
            DTK_P();

            p.contextObject.reset(new qt::ContextObject(_context));

            p.settingsModel = play::SettingsModel::create(_context, p.settings);

            p.timeUnitsModel = timeline::TimeUnitsModel::create(_context);

            p.timeObject.reset(new qt::TimeObject(p.timeUnitsModel));

            p.filesModel = play::FilesModel::create(_context);

            p.recentFilesModel = play::RecentFilesModel::create(_context, p.settings);

            p.colorModel = play::ColorModel::create(_context);
            p.colorModel->setOCIOOptions(p.options.ocioOptions);
            p.colorModel->setLUTOptions(p.options.lutOptions);

            p.viewportModel = play::ViewportModel::create(_context, p.settings);

            p.renderModel = play::RenderModel::create(_context, p.settings);

            p.audioModel = play::AudioModel::create(_context, p.settings);
        }

        void App::_devicesInit()
        {
            DTK_P();
#if defined(TLRENDER_BMD)
            p.bmdOutputDevice = bmd::OutputDevice::create(_context);
            if (0)
            {
                auto overlayImage = dtk::Image::create(
                    1920,
                    1080,
                    dtk::ImageType::ARGB_4444_Premult);
                QImage* bmdOverlayImage = new QImage(
                    overlayImage->getData(),
                    1920,
                    1080,
                    QImage::Format_ARGB4444_Premultiplied);
                bmdOverlayImage->fill(QColor(0, 0, 255, 63));
                p.bmdOutputDevice->setOverlay(overlayImage);
            }
            p.bmdDevicesModel = play::BMDDevicesModel::create(_context, p.settings);
#endif // TLRENDER_BMD
        }

        void App::_observersInit()
        {
            DTK_P();

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
                    if (_p->player)
                    {
                        _p->player->setCompareTime(value);
                    }
                });

            p.audioDeviceObserver = dtk::ValueObserver<audio::DeviceID>::create(
                p.audioModel->observeDevice(),
                [this](const audio::DeviceID& value)
                {
                    if (auto player = _p->player)
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
                    p.bmdOutputDevice->setDisplayOptions({ displayOptions });
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

            p.compareOptionsObserver = dtk::ValueObserver<timeline::CompareOptions>::create(
                p.filesModel->observeCompareOptions(),
                [this](const timeline::CompareOptions& value)
                {
                    _p->bmdOutputDevice->setCompareOptions(value);
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
            DTK_P();

            p.mainWindow.reset(new MainWindow(this));
            p.mainWindow->show();

            connect(
                p.mainWindow.get(),
                &QObject::destroyed,
                [this](QObject*)
                {
                    _p->mainWindow.take();
                    if (_p->secondaryWindow)
                    {
                        _p->secondaryWindow->close();
                    }
                });

            p.mainWindow->viewport()->setViewPosAndZoomCallback(
                [this](const dtk::V2I& pos, double zoom)
                {
                    _viewUpdate(
                        pos,
                        zoom,
                        _p->mainWindow->viewport()->hasFrameView());
                });
            p.mainWindow->viewport()->setFrameViewCallback(
                [this](bool value)
                {
                    _viewUpdate(
                        _p->mainWindow->viewport()->getViewPos(),
                        _p->mainWindow->viewport()->getViewZoom(),
                        value);
                });
        }

        io::Options App::_ioOptions() const
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

        void App::_timerUpdate()
        {
#if defined(TLRENDER_BMD)
            if (_p && _p->bmdOutputDevice)
            {
                _p->bmdOutputDevice->tick();
            }
#endif // TLRENDER_BMD
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
                        options.ioOptions = _ioOptions();
                        options.pathOptions.maxNumberDigits = fileSequence.maxDigits;
                        auto otioTimeline = files[i]->audioPath.isEmpty() ?
                            timeline::create(_context, files[i]->path, options) :
                            timeline::create(_context, files[i]->path, files[i]->audioPath, options);
                        if (0)
                        {
                            timeline::toMemoryReferences(
                                otioTimeline,
                                files[i]->path.getDirectory(),
                                timeline::ToMemoryReference::Shared,
                                options.pathOptions);
                        }
                        timelines[i] = timeline::Timeline::create(_context, otioTimeline, options);
                        for (const auto& video : timelines[i]->getIOInfo().video)
                        {
                            files[i]->videoLayers.push_back(video.name);
                        }
                    }
                    catch (const std::exception& e)
                    {
                        _context->log("tl::play_qt::App", e.what(), dtk::LogType::Error);
                    }
                }
            }

            p.files = files;
            p.timelines = timelines;
        }

        void App::_activeUpdate(const std::vector<std::shared_ptr<play::FilesModelItem> >& activeFiles)
        {
            DTK_P();

            if (!p.activeFiles.empty() && p.player)
            {
                p.activeFiles.front()->currentTime = p.player->currentTime();
            }

            QSharedPointer<qt::TimelinePlayer> player;
            if (!activeFiles.empty())
            {
                if (!p.activeFiles.empty() && activeFiles[0] == p.activeFiles[0])
                {
                    player = p.player;
                }
                else
                {
                    if (p.player)
                    {
                        p.player->setAudioDevice(audio::DeviceID());
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
                                player.reset(new qt::TimelinePlayer(
                                    _context,
                                    timeline::Player::create(_context, timeline, playerOptions),
                                    this));
                            }
                            catch (const std::exception& e)
                            {
                                _context->log("tl::play_qt::App", e.what(), dtk::LogType::Error);
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
            p.player = player;
#if defined(TLRENDER_BMD)
            p.bmdOutputDevice->setPlayer(p.player ? p.player->player() : nullptr);
#endif // TLRENDER_BMD

            _layersUpdate(p.filesModel->observeLayers()->get());
            _cacheUpdate(p.settingsModel->getCache());
            _audioUpdate();

            Q_EMIT playerChanged(p.player);
        }

        void App::_layersUpdate(const std::vector<int>& value)
        {
            DTK_P();
            if (auto player = p.player)
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
            if (p.player)
            {
                p.player->setCacheOptions(cacheOptions);
            }
        }

        void App::_viewUpdate(const dtk::V2I& pos, double zoom, bool frame)
        {
            DTK_P();
            float scale = 1.F;
            const dtk::Size2I& size = p.mainWindow->viewport()->getGeometry().size() *
                p.mainWindow->devicePixelRatio();
            if (p.secondaryWindow)
            {
                const QSize& secondarySize = p.secondaryWindow->size() *
                    p.secondaryWindow->devicePixelRatio();
                if (size.isValid() && secondarySize.isValid())
                {
                    scale = secondarySize.width() / static_cast<float>(size.w);
                }
                p.secondaryWindow->setView(pos * scale, zoom * scale, frame);
            }
#if defined(TLRENDER_BMD)
            scale = 1.F;
            const dtk::Size2I& bmdSize = p.bmdOutputDevice->getSize();
            if (size.isValid() && bmdSize.isValid())
            {
                scale = bmdSize.w / static_cast<float>(size.w);
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
            if (p.player)
            {
                p.player->setVolume(volume);
                p.player->setMute(mute || p.bmdDeviceActive);
                p.player->setChannelMute(channelMute);
                p.player->setAudioOffset(audioOffset);
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
