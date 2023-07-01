// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2023 Darby Johnston
// All rights reserved.

#include <tlPlayQtApp/App.h>

#include <tlPlayQtApp/ColorModel.h>
#include <tlPlayQtApp/DevicesModel.h>
#include <tlPlayQtApp/MainWindow.h>
#include <tlPlayQtApp/MemoryTimeline.h>
#include <tlPlayQtApp/OpenSeparateAudioDialog.h>
#include <tlPlayQtApp/SettingsObject.h>

#include <tlQtWidget/Init.h>
#include <tlQtWidget/Style.h>

#include <tlQt/ContextObject.h>
#include <tlQt/MetaTypes.h>
#include <tlQt/OutputDevice.h>
#include <tlQt/TimeObject.h>
#include <tlQt/TimelineThumbnailObject.h>
#include <tlQt/TimelinePlayer.h>

#include <tlPlay/FilesModel.h>

#include <tlTimeline/Util.h>

#include <tlIO/IOSystem.h>
#if defined(TLRENDER_USD)
#include <tlIO/USD.h>
#endif // TLRENDER_USD

#include <tlCore/AudioSystem.h>
#include <tlCore/Math.h>
#include <tlCore/StringFormat.h>
#include <tlCore/Time.h>

#include <QFileDialog>

namespace tl
{
    namespace play_qt
    {
        namespace
        {
            struct Options
            {
                std::string fileName;
                std::string audioFileName;
                std::string compareFileName;
                timeline::CompareOptions compareOptions;
                double speed = 0.0;
                timeline::Playback playback = timeline::Playback::Stop;
                timeline::Loop loop = timeline::Loop::Loop;
                otime::RationalTime seek = time::invalidTime;
                otime::TimeRange inOutRange = time::invalidTimeRange;
                timeline::ColorConfigOptions colorConfigOptions;
                timeline::LUTOptions lutOptions;
                bool resetSettings = false;
#if defined(TLRENDER_USD)
                size_t usdRenderWidth = usd::RenderOptions().renderWidth;
                float usdComplexity = usd::RenderOptions().complexity;
                usd::DrawMode usdDrawMode = usd::RenderOptions().drawMode;
                bool usdEnableLighting = usd::RenderOptions().enableLighting;
                size_t usdStageCache = usd::RenderOptions().stageCacheCount;
                size_t usdDiskCache = usd::RenderOptions().diskCacheByteCount / memory::gigabyte;
#endif // TLRENDER_USD
            };
        }

        struct App::Private
        {
            Options options;

            qt::ContextObject* contextObject = nullptr;
            std::shared_ptr<timeline::TimeUnitsModel> timeUnitsModel;
            qt::TimeObject* timeObject = nullptr;
            SettingsObject* settingsObject = nullptr;
            qt::TimelineThumbnailObject* thumbnailObject = nullptr;
            std::shared_ptr<play::FilesModel> filesModel;
            std::vector<std::shared_ptr<play::FilesModelItem> > files;
            std::vector<std::shared_ptr<play::FilesModelItem> > activeFiles;
            std::shared_ptr<observer::ListObserver<std::shared_ptr<play::FilesModelItem> > > filesObserver;
            std::shared_ptr<observer::ListObserver<std::shared_ptr<play::FilesModelItem> > > activeObserver;
            std::shared_ptr<observer::ListObserver<int> > layersObserver;
            std::shared_ptr<ColorModel> colorModel;
            timeline::LUTOptions lutOptions;
            timeline::ImageOptions imageOptions;
            timeline::DisplayOptions displayOptions;
            float volume = 1.F;
            bool mute = false;
            qt::OutputDevice* outputDevice = nullptr;
            bool deviceActive = false;
            std::shared_ptr<DevicesModel> devicesModel;
            std::shared_ptr<observer::ValueObserver<DevicesModelData> > devicesObserver;

            QVector<QPointer<qt::TimelinePlayer> > players;

            MainWindow* mainWindow = nullptr;
        };

        App::App(
            int& argc,
            char** argv,
            const std::shared_ptr<system::Context>& context) :
            QApplication(argc, argv),
            _p(new Private)
        {
            TLRENDER_P();

            IApp::_init(
                argc,
                argv,
                context,
                "tlplay-qt",
                "Play timelines, movies, and image sequences.",
                {
                    app::CmdLineValueArg<std::string>::create(
                        p.options.fileName,
                        "input",
                        "Timeline, movie, image sequence, or folder.",
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
                        { "-loop" },
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
                        "Color configuration file name (config.ocio)."),
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
                    app::CmdLineFlagOption::create(
                        p.options.resetSettings,
                        { "-resetSettings" },
                        "Reset settings to defaults."),
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
                        "USD disk cache size in gigabytes. A size of zero disables the cache.",
                        string::Format("{0}").arg(p.options.usdDiskCache)),
#endif // TLRENDER_USD
                });
            const int exitCode = getExit();
            if (exitCode != 0)
            {
                exit(exitCode);
                return;
            }

            // Initialize Qt.
            setOrganizationName("tlRender");
            setApplicationName("tlplay-qt");
            setStyle("Fusion");
            setPalette(qtwidget::darkStyle());
            setStyleSheet(qtwidget::styleSheet());
            qtwidget::initFonts(context);

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

            // Create models and objects.
            p.contextObject = new qt::ContextObject(context, this);
            p.timeUnitsModel = timeline::TimeUnitsModel::create(context);
            p.timeObject = new qt::TimeObject(p.timeUnitsModel, this);

            p.settingsObject = new SettingsObject(p.options.resetSettings, p.timeObject, this);
            connect(
                p.settingsObject,
                &SettingsObject::valueChanged,
                [this](const QString& name, const QVariant&)
                {
                    if ("Cache/ReadAhead" == name ||
                        "Cache/ReadBehind" == name)
                    {
                        _cacheUpdate();
                    }
                });

            p.thumbnailObject = new qt::TimelineThumbnailObject(context, this);

            p.filesModel = play::FilesModel::create(context);
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
                            _p->players[i]->setVideoLayer(value[i]);
                        }
                    }
                });

            p.colorModel = ColorModel::create(context);
            if (!p.options.colorConfigOptions.fileName.empty())
            {
                p.colorModel->setConfigOptions(p.options.colorConfigOptions);
            }

            p.lutOptions = p.options.lutOptions;

            p.outputDevice = new qt::OutputDevice(context);
            connect(
                p.outputDevice,
                &qt::OutputDevice::deviceActiveChanged,
                [this](bool value)
                {
                    _p->deviceActive = value;
                    _audioUpdate();
                });
            /*connect(
                p.outputDevice,
                &qt::OutputDevice::sizeChanged,
                [this](const imaging::Size& value)
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
            p.devicesModel = DevicesModel::create(context);
            p.devicesModel->setDeviceIndex(p.settingsObject->value("Devices/DeviceIndex").toInt());
            p.devicesModel->setDisplayModeIndex(p.settingsObject->value("Devices/DisplayModeIndex").toInt());
            p.devicesModel->setPixelTypeIndex(p.settingsObject->value("Devices/PixelTypeIndex").toInt());
            p.devicesModel->setDeviceEnabled(p.settingsObject->value("Devices/DeviceEnabled").toBool());
            p.settingsObject->setDefaultValue("Devices/HDRMode",\
                static_cast<int>(device::HDRMode::FromFile));
            p.devicesModel->setHDRMode(
                static_cast<device::HDRMode>(p.settingsObject->value("Devices/HDRMode").toInt()));
            std::string s = p.settingsObject->value("Devices/HDRData").toString().toUtf8().data();
            if (!s.empty())
            {
                auto json = nlohmann::json::parse(s);
                imaging::HDRData hdrData;
                from_json(json, hdrData);
                p.devicesModel->setHDRData(hdrData);
            }
            p.devicesObserver = observer::ValueObserver<DevicesModelData>::create(
                p.devicesModel->observeData(),
                [this](const DevicesModelData& value)
                {
                    const device::PixelType pixelType = value.pixelTypeIndex >= 0 &&
                        value.pixelTypeIndex < value.pixelTypes.size() ?
                        value.pixelTypes[value.pixelTypeIndex] :
                        device::PixelType::None;
                    _p->outputDevice->setDevice(
                        value.deviceIndex - 1,
                        value.displayModeIndex - 1,
                        pixelType);
                    _p->outputDevice->setDeviceEnabled(value.deviceEnabled);
                    _p->outputDevice->setHDR(value.hdrMode, value.hdrData);
                });

            // Create the main window.
            p.mainWindow = new MainWindow(this);

            // Open the input files.
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

            p.mainWindow->show();
        }

        App::~App()
        {
            TLRENDER_P();

            delete p.mainWindow;
            p.mainWindow = nullptr;

            delete p.outputDevice;
            p.outputDevice = nullptr;

            if (p.settingsObject && p.devicesModel)
            {
                const auto& deviceData = p.devicesModel->observeData()->get();
                p.settingsObject->setValue("Devices/DeviceIndex", deviceData.deviceIndex);
                p.settingsObject->setValue("Devices/DisplayModeIndex", deviceData.displayModeIndex);
                p.settingsObject->setValue("Devices/PixelTypeIndex", deviceData.pixelTypeIndex);
                p.settingsObject->setValue("Devices/DeviceEnabled", deviceData.deviceEnabled);
                p.settingsObject->setValue("Devices/HDRMode", static_cast<int>(deviceData.hdrMode));
                nlohmann::json json;
                to_json(json, deviceData.hdrData);
                p.settingsObject->setValue("Devices/HDRData", QString::fromUtf8(json.dump().c_str()));
            }

            //! \bug Why is it necessary to manually delete this to get the settings to save?
            delete p.settingsObject;
            p.settingsObject = nullptr;
        }

        const std::shared_ptr<timeline::TimeUnitsModel>& App::timeUnitsModel() const
        {
            return _p->timeUnitsModel;
        }

        qt::TimeObject* App::timeObject() const
        {
            return _p->timeObject;
        }

        SettingsObject* App::settingsObject() const
        {
            return _p->settingsObject;
        }

        qt::TimelineThumbnailObject* App::thumbnailObject() const
        {
            return _p->thumbnailObject;
        }

        const std::shared_ptr<play::FilesModel>& App::filesModel() const
        {
            return _p->filesModel;
        }

        const std::shared_ptr<ColorModel>& App::colorModel() const
        {
            return _p->colorModel;
        }

        const timeline::LUTOptions& App::lutOptions() const
        {
            return _p->lutOptions;
        }

        const timeline::ImageOptions& App::imageOptions() const
        {
            return _p->imageOptions;
        }

        const timeline::DisplayOptions& App::displayOptions() const
        {
            return _p->displayOptions;
        }

        float App::volume() const
        {
            return _p->volume;
        }

        bool App::isMuted() const
        {
            return _p->mute;
        }

        qt::OutputDevice* App::outputDevice() const
        {
            return _p->outputDevice;
        }

        const std::shared_ptr<DevicesModel>& App::devicesModel() const
        {
            return _p->devicesModel;
        }

        void App::open(const QString& fileName, const QString& audioFileName)
        {
            TLRENDER_P();
            file::PathOptions pathOptions;
            pathOptions.maxNumberDigits = p.settingsObject->value("Misc/MaxFileSequenceDigits").toInt();
            for (const auto& path : timeline::getPaths(fileName.toUtf8().data(), pathOptions, _context))
            {
                auto item = std::make_shared<play::FilesModelItem>();
                item->path = path;
                item->audioPath = file::Path(audioFileName.toUtf8().data());
                p.filesModel->add(item);
                p.settingsObject->addRecentFile(QString::fromUtf8(path.get().c_str()));
            }
        }

        void App::openDialog()
        {
            TLRENDER_P();

            std::vector<std::string> extensions;
            for (const auto& i : timeline::getExtensions(
                static_cast<int>(io::FileType::Movie) |
                static_cast<int>(io::FileType::Sequence) |
                static_cast<int>(io::FileType::Audio),
                _context))
            {
                extensions.push_back("*" + i);
            }

            QString dir;
            if (!p.activeFiles.empty())
            {
                dir = QString::fromUtf8(p.activeFiles[0]->path.get().c_str());
            }

            const auto fileName = QFileDialog::getOpenFileName(
                p.mainWindow,
                tr("Open"),
                dir,
                tr("Files") + " (" + QString::fromUtf8(string::join(extensions, " ").c_str()) + ")");
            if (!fileName.isEmpty())
            {
                open(fileName);
            }
        }

        void App::openSeparateAudioDialog()
        {
            auto dialog = std::make_unique<OpenSeparateAudioDialog>(_context);
            if (QDialog::Accepted == dialog->exec())
            {
                open(dialog->videoFileName(), dialog->audioFileName());
            }
        }

        void App::setLUTOptions(const timeline::LUTOptions& value)
        {
            TLRENDER_P();
            if (value == p.lutOptions)
                return;
            p.lutOptions = value;
            Q_EMIT lutOptionsChanged(p.lutOptions);
        }

        void App::setImageOptions(const timeline::ImageOptions& value)
        {
            TLRENDER_P();
            if (value == p.imageOptions)
                return;
            p.imageOptions = value;
            Q_EMIT imageOptionsChanged(p.imageOptions);
        }

        void App::setDisplayOptions(const timeline::DisplayOptions& value)
        {
            TLRENDER_P();
            if (value == p.displayOptions)
                return;
            p.displayOptions = value;
            Q_EMIT displayOptionsChanged(p.displayOptions);
        }

        void App::setVolume(float value)
        {
            TLRENDER_P();
            if (value == p.volume)
                return;
            p.volume = value;
            _audioUpdate();
            Q_EMIT volumeChanged(p.volume);
        }

        void App::setMute(bool value)
        {
            TLRENDER_P();
            if (value == p.mute)
                return;
            p.mute = value;
            _audioUpdate();
            Q_EMIT muteChanged(p.mute);
        }

        void App::_filesCallback(const std::vector<std::shared_ptr<play::FilesModelItem> >& items)
        {
            TLRENDER_P();

            // Create the new list of players.
            QVector<QPointer<qt::TimelinePlayer> > players(items.size(), nullptr);
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
            QVector<QPointer<qt::TimelinePlayer> > destroy;
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
                        options.fileSequenceAudio = p.settingsObject->value("FileSequence/Audio").
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
                            255);
                        auto otioTimeline = items[i]->audioPath.isEmpty() ?
                            timeline::create(items[i]->path.get(), _context, options) :
                            timeline::create(items[i]->path.get(), items[i]->audioPath.get(), _context, options);
                        if (0)
                        {
                            createMemoryTimeline(otioTimeline, items[i]->path.getDirectory(), options.pathOptions);
                        }
                        auto timeline = timeline::Timeline::create(otioTimeline, _context, options);
                        const otime::TimeRange& timeRange = timeline->getTimeRange();

                        timeline::PlayerOptions playerOptions;
                        playerOptions.cache.readAhead = time::invalidTime;
                        playerOptions.cache.readBehind = time::invalidTime;
                        playerOptions.timerMode = p.settingsObject->value("Performance/TimerMode").
                            value<timeline::TimerMode>();
                        playerOptions.audioBufferFrameCount =
                            p.settingsObject->value("Performance/AudioBufferFrameCount").toInt();
                        auto player = timeline::Player::create(timeline, _context, playerOptions);
                        players[i] = new qt::TimelinePlayer(player, _context, this);

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
            for (auto i :destroy)
            {
                delete i;
            }
        }

        void App::_activeCallback(const std::vector<std::shared_ptr<play::FilesModelItem> >& items)
        {
            TLRENDER_P();

            auto activePlayers = _activePlayers();
            if (!activePlayers.empty() && activePlayers[0])
            {
                activePlayers[0]->setPlayback(timeline::Playback::Stop);

                disconnect(
                    activePlayers[0],
                    SIGNAL(audioOffsetChanged(double)),
                    this,
                    SLOT(_audioOffsetCallback(double)));
            }

            p.activeFiles = items;

            activePlayers = _activePlayers();
            if (p.mainWindow)
            {
                p.mainWindow->setTimelinePlayers(activePlayers);
            }
            qt::TimelinePlayer* first = nullptr;
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

            _cacheUpdate();
            _audioUpdate();
        }

        void App::_audioOffsetCallback(double)
        {
            _audioUpdate();
        }

        QVector<QPointer<qt::TimelinePlayer> > App::_activePlayers() const
        {
            TLRENDER_P();
            QVector<QPointer<qt::TimelinePlayer> > out;
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

        otime::RationalTime App::_cacheReadAhead() const
        {
            TLRENDER_P();
            const size_t activeCount = p.filesModel->observeActive()->getSize();
            return otime::RationalTime(
                p.settingsObject->value("Cache/ReadAhead").toDouble() / static_cast<double>(activeCount),
                1.0);
        }

        otime::RationalTime App::_cacheReadBehind() const
        {
            TLRENDER_P();
            const size_t activeCount = p.filesModel->observeActive()->getSize();
            return otime::RationalTime(
                p.settingsObject->value("Cache/ReadBehind").toDouble() / static_cast<double>(activeCount),
                1.0);
        }

        void App::_cacheUpdate()
        {
            TLRENDER_P();

            const auto activePlayers = _activePlayers();

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
                player->setCacheOptions(cacheOptions);
            }
        }

        void App::_audioUpdate()
        {
            TLRENDER_P();
            for (auto player : p.players)
            {
                if (player)
                {
                    player->setVolume(p.volume);
                    player->setMute(p.mute || p.deviceActive);
                }
            }
            if (p.outputDevice)
            {
                p.outputDevice->setVolume(p.volume);
                p.outputDevice->setMute(p.mute);
                const auto activePlayers = _activePlayers();
                p.outputDevice->setAudioOffset(
                    (!activePlayers.empty() && activePlayers[0]) ?
                    activePlayers[0]->audioOffset() :
                    0.0);
            }
        }
    }
}
