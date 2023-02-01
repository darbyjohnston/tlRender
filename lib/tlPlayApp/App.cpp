// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2023 Darby Johnston
// All rights reserved.

#include <tlPlayApp/App.h>

#include <tlPlayApp/ColorModel.h>
#include <tlPlayApp/DevicesModel.h>
#include <tlPlayApp/FilesModel.h>
#include <tlPlayApp/MainWindow.h>
#include <tlPlayApp/MemoryTimeline.h>
#include <tlPlayApp/OpenSeparateAudioDialog.h>
#include <tlPlayApp/SettingsObject.h>

#include <tlQtWidget/Style.h>
#include <tlQtWidget/Util.h>

#include <tlQt/ContextObject.h>
#include <tlQt/MetaTypes.h>
#include <tlQt/OutputDevice.h>
#include <tlQt/TimeObject.h>
#include <tlQt/TimelineThumbnailProvider.h>
#include <tlQt/TimelinePlayer.h>

#include <tlTimeline/Util.h>

#include <tlIO/IOSystem.h>

#include <tlCore/AudioSystem.h>
#include <tlCore/Math.h>
#include <tlCore/StringFormat.h>
#include <tlCore/Time.h>

#include <QFileDialog>

namespace tl
{
    namespace play
    {
        namespace
        {
            struct Options
            {
                std::string fileName;
                std::string audioFileName;
                std::string compareFileName;
                timeline::CompareMode compareMode = timeline::CompareMode::A;
                math::Vector2f wipeCenter = math::Vector2f(.5F, .5F);
                float wipeRotation = 0.F;
                double speed = 0.0;
                timeline::Playback playback = timeline::Playback::Stop;
                timeline::Loop loop = timeline::Loop::Loop;
                otime::RationalTime seek = time::invalidTime;
                otime::TimeRange inOutRange = time::invalidTimeRange;
                timeline::ColorConfigOptions colorConfigOptions;
                timeline::LUTOptions lutOptions;
                bool resetSettings = false;
            };
        }

        struct App::Private
        {
            Options options;

            qt::ContextObject* contextObject = nullptr;
            qt::TimeObject* timeObject = nullptr;
            SettingsObject* settingsObject = nullptr;
            qt::TimelineThumbnailProvider* thumbnailProvider = nullptr;
            std::shared_ptr<FilesModel> filesModel;
            std::shared_ptr<observer::ListObserver<std::shared_ptr<FilesModelItem> > > activeObserver;
            std::vector<std::shared_ptr<FilesModelItem> > active;
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

            std::vector<qt::TimelinePlayer*> timelinePlayers;

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
                "tlplay",
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
                p.options.compareMode,
                { "-compareMode", "-c" },
                "A/B comparison mode.",
                string::Format("{0}").arg(p.options.compareMode),
                string::join(timeline::getCompareModeLabels(), ", ")),
            app::CmdLineValueOption<math::Vector2f>::create(
                p.options.wipeCenter,
                { "-wipeCenter", "-wc" },
                "A/B comparison wipe center.",
                string::Format("{0}").arg(p.options.wipeCenter)),
            app::CmdLineValueOption<float>::create(
                p.options.wipeRotation,
                { "-wipeRotation", "-wr" },
                "A/B comparison wipe rotation.",
                string::Format("{0}").arg(p.options.wipeRotation)),
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
                "Reset settings to defaults.")
        });
            const int exitCode = getExit();
            if (exitCode != 0)
            {
                exit(exitCode);
                return;
            }

            // Initialize Qt.
            setOrganizationName("tlRender");
            setApplicationName("tlplay");
            setStyle("Fusion");
            setPalette(qtwidget::darkStyle());
            setStyleSheet(qtwidget::styleSheet());
            qtwidget::initFonts(context);

            // Create objects.
            p.contextObject = new qt::ContextObject(context, this);
            p.timeObject = new qt::TimeObject(this);

            p.settingsObject = new SettingsObject(p.options.resetSettings, p.timeObject, this);
            connect(
                p.settingsObject,
                &SettingsObject::valueChanged,
                [this](const QString& name, const QVariant&)
                {
                    if ("Cache/ReadAhead" == name ||
                        "Cache/ReadBehind" == name ||
                        "Performance/VideoRequestCount" == name ||
                        "Performance/AudioRequestCount" == name ||
                        "Performance/SequenceThreadCount" == name ||
                        "Performance/FFmpegThreadCount")
                    {
                        _cacheUpdate();
                    }
                });

            p.thumbnailProvider = new qt::TimelineThumbnailProvider(context, this);

            p.filesModel = FilesModel::create(context);
            p.activeObserver = observer::ListObserver<std::shared_ptr<FilesModelItem> >::create(
                p.filesModel->observeActive(),
                [this](const std::vector<std::shared_ptr<FilesModelItem> >& value)
                {
                    _activeCallback(value);
                });
            p.layersObserver = observer::ListObserver<int>::create(
                p.filesModel->observeLayers(),
                [this](const std::vector<int>& value)
                {
                    for (size_t i = 0; i < value.size() && i < _p->timelinePlayers.size(); ++i)
                    {
                        if (_p->timelinePlayers[i])
                        {
                            _p->timelinePlayers[i]->setVideoLayer(value[i]);
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

            _cacheUpdate();
            _audioUpdate();

            // Create the main window.
            p.mainWindow = new MainWindow(this);

            // Open the input files.
            if (!p.options.fileName.empty())
            {
                if (!p.options.compareFileName.empty())
                {
                    timeline::CompareOptions compareOptions;
                    compareOptions.mode = p.options.compareMode;
                    compareOptions.wipeCenter = p.options.wipeCenter;
                    compareOptions.wipeRotation = p.options.wipeRotation;
                    p.filesModel->setCompareOptions(compareOptions);
                    open(QString::fromUtf8(p.options.compareFileName.c_str()));
                }

                open(
                    QString::fromUtf8(p.options.fileName.c_str()),
                    QString::fromUtf8(p.options.audioFileName.c_str()));

                if (!p.timelinePlayers.empty() && p.timelinePlayers[0])
                {
                    if (p.options.speed > 0.0)
                    {
                        p.timelinePlayers[0]->setSpeed(p.options.speed);
                    }
                    if (time::isValid(p.options.inOutRange))
                    {
                        p.timelinePlayers[0]->setInOutRange(p.options.inOutRange);
                        p.timelinePlayers[0]->seek(p.options.inOutRange.start_time());
                    }
                    if (time::isValid(p.options.seek))
                    {
                        p.timelinePlayers[0]->seek(p.options.seek);
                    }
                    p.timelinePlayers[0]->setLoop(p.options.loop);
                    p.timelinePlayers[0]->setPlayback(p.options.playback);
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

        qt::TimeObject* App::timeObject() const
        {
            return _p->timeObject;
        }

        SettingsObject* App::settingsObject() const
        {
            return _p->settingsObject;
        }

        qt::TimelineThumbnailProvider* App::thumbnailProvider() const
        {
            return _p->thumbnailProvider;
        }

        const std::shared_ptr<FilesModel>& App::filesModel() const
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
                auto item = std::make_shared<FilesModelItem>();
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
            if (!p.active.empty())
            {
                dir = QString::fromUtf8(p.active[0]->path.get().c_str());
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

        void App::_activeCallback(const std::vector<std::shared_ptr<FilesModelItem> >& items)
        {
            TLRENDER_P();

            // Save the previous timeline player state.
            if (!p.active.empty() && !p.timelinePlayers.empty())
            {
                p.active[0]->speed = p.timelinePlayers[0]->speed();
                p.active[0]->playback = p.timelinePlayers[0]->playback();
                p.active[0]->loop = p.timelinePlayers[0]->loop();
                p.active[0]->currentTime = p.timelinePlayers[0]->currentTime();
                p.active[0]->inOutRange = p.timelinePlayers[0]->inOutRange();
                p.active[0]->videoLayer = p.timelinePlayers[0]->videoLayer();
                p.active[0]->audioOffset = p.timelinePlayers[0]->audioOffset();
            }

            // Create new timeline players.
            std::vector<qt::TimelinePlayer*> newTimelinePlayers;
            auto audioSystem = _context->getSystem<audio::System>();
            for (size_t i = 0; i < items.size(); ++i)
            {
                const auto& item = items[i];
                qt::TimelinePlayer* qtTimelinePlayer = nullptr;
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
                    options.ioOptions["ffmpeg/YUVToRGBConversion"] = string::Format("{0}").
                        arg(p.settingsObject->value("Performance/FFmpegYUVToRGBConversion").toBool());
                    options.ioOptions["ffmpeg/ThreadCount"] = string::Format("{0}").
                        arg(p.settingsObject->value("Performance/FFmpegThreadCount").toInt());
                    options.pathOptions.maxNumberDigits = std::min(
                        p.settingsObject->value("Misc/MaxFileSequenceDigits").toInt(),
                        255);
                    auto otioTimeline = item->audioPath.isEmpty() ?
                        timeline::create(item->path.get(), _context, options) :
                        timeline::create(item->path.get(), item->audioPath.get(), _context, options);
                    if (0)
                    {
                        createMemoryTimeline(otioTimeline, item->path.getDirectory(), options.pathOptions);
                    }
                    auto timeline = timeline::Timeline::create(otioTimeline, _context, options);

                    timeline::PlayerOptions playerOptions;
                    playerOptions.cache.readAhead = _cacheReadAhead();
                    playerOptions.cache.readBehind = _cacheReadBehind();
                    playerOptions.timerMode = p.settingsObject->value("Performance/TimerMode").
                        value<timeline::TimerMode>();
                    playerOptions.audioBufferFrameCount = p.settingsObject->value("Performance/AudioBufferFrameCount").
                        value<timeline::AudioBufferFrameCount>();
                    if (item->init)
                    {
                        playerOptions.currentTime = items[0]->currentTime;
                    }
                    auto timelinePlayer = timeline::TimelinePlayer::create(timeline, _context, playerOptions);
                    qtTimelinePlayer = new qt::TimelinePlayer(timelinePlayer, _context, this);
                    item->timeRange = qtTimelinePlayer->timeRange();
                    item->ioInfo = qtTimelinePlayer->ioInfo();
                    if (!item->init)
                    {
                        item->init = true;
                        item->speed = qtTimelinePlayer->speed();
                        item->playback = qtTimelinePlayer->playback();
                        item->loop = qtTimelinePlayer->loop();
                        item->currentTime = qtTimelinePlayer->currentTime();
                        item->inOutRange = qtTimelinePlayer->inOutRange();
                        item->videoLayer = qtTimelinePlayer->videoLayer();
                        item->audioOffset = qtTimelinePlayer->audioOffset();
                    }
                    else if (0 == i)
                    {
                        qtTimelinePlayer->setSpeed(items[0]->speed);
                        qtTimelinePlayer->setLoop(items[0]->loop);
                        qtTimelinePlayer->setInOutRange(items[0]->inOutRange);
                        qtTimelinePlayer->setVideoLayer(items[0]->videoLayer);
                        qtTimelinePlayer->setVolume(p.volume);
                        qtTimelinePlayer->setMute(p.mute);
                        qtTimelinePlayer->setAudioOffset(items[0]->audioOffset);

                        qtTimelinePlayer->setPlayback(items[0]->playback);
                    }
                    if (i > 0)
                    {
                        qtTimelinePlayer->setVideoLayer(items[i]->videoLayer);
                        qtTimelinePlayer->timelinePlayer()->setExternalTime(
                            newTimelinePlayers[0]->timelinePlayer());
                    }
                }
                catch (const std::exception& e)
                {
                    _log(e.what(), log::Type::Error);
                }
                newTimelinePlayers.push_back(qtTimelinePlayer);
            }

            // Connect signals.
            if (!newTimelinePlayers.empty())
            {
                connect(
                    newTimelinePlayers[0],
                    &qt::TimelinePlayer::audioOffsetChanged,
                    [this](double)
                    {
                        _audioUpdate();
                    });
            }

            // Set the main window timeline players.
            if (p.mainWindow)
            {
                std::vector<qt::TimelinePlayer*> validTimelinePlayers;
                for (const auto& i : newTimelinePlayers)
                {
                    if (i)
                    {
                        validTimelinePlayers.push_back(i);
                    }
                }
                p.mainWindow->setTimelinePlayers(validTimelinePlayers);
            }

            // Delete the previous timeline players.
            for (size_t i = 0; i < p.timelinePlayers.size(); ++i)
            {
                delete p.timelinePlayers[i];
            }

            p.active = items;
            p.timelinePlayers = newTimelinePlayers;

            _cacheUpdate();
            _audioUpdate();
        }

        void App::_settingsCallback()
        {
            _cacheUpdate();
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
            timeline::PlayerCacheOptions options;
            options.readAhead = _cacheReadAhead();
            options.readBehind = _cacheReadBehind();
            for (const auto& i : p.timelinePlayers)
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
            for (const auto& i : p.timelinePlayers)
            {
                if (i)
                {
                    i->setVolume(p.volume);
                    i->setMute(p.mute || p.deviceActive);
                }
            }
            if (p.outputDevice)
            {
                p.outputDevice->setVolume(p.volume);
                p.outputDevice->setMute(p.mute);
                p.outputDevice->setAudioOffset(
                    !p.timelinePlayers.empty() ?
                    p.timelinePlayers[0]->audioOffset() :
                    0.0);
            }
        }
    }
}
