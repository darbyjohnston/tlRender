// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2022 Darby Johnston
// All rights reserved.

#include <tlPlayApp/App.h>

#include <tlPlayApp/ColorModel.h>
#include <tlPlayApp/DevicesModel.h>
#include <tlPlayApp/FilesModel.h>
#include <tlPlayApp/MainWindow.h>
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
            qt::OutputDevice* outputDevice = nullptr;
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
            QCoreApplication::setOrganizationName("tlRender");
            QCoreApplication::setApplicationName("tlplay");
            setStyle("Fusion");
            setPalette(qtwidget::darkStyle());
            setStyleSheet(qtwidget::styleSheet());

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

            _cacheUpdate();

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
            p.devicesModel = DevicesModel::create(context);
            p.devicesModel->setDeviceIndex(p.settingsObject->value("Devices/DeviceIndex").toInt());
            p.devicesModel->setDisplayModeIndex(p.settingsObject->value("Devices/DisplayModeIndex").toInt());
            p.devicesModel->setPixelTypeIndex(p.settingsObject->value("Devices/PixelTypeIndex").toInt());
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
                    _p->outputDevice->setHDR(value.hdrMode, value.hdrData);
                });

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
                    if (p.options.inOutRange != time::invalidTimeRange)
                    {
                        p.timelinePlayers[0]->setInOutRange(p.options.inOutRange);
                        p.timelinePlayers[0]->seek(p.options.inOutRange.start_time());
                    }
                    if (p.options.seek != time::invalidTime)
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

        void App::_activeCallback(const std::vector<std::shared_ptr<FilesModelItem> >& items)
        {
            TLRENDER_P();

            if (!p.active.empty() &&
                !p.timelinePlayers.empty() &&
                p.timelinePlayers[0])
            {
                p.active[0]->init = true;
                p.active[0]->speed = p.timelinePlayers[0]->speed();
                p.active[0]->playback = p.timelinePlayers[0]->playback();
                p.active[0]->loop = p.timelinePlayers[0]->loop();
                p.active[0]->currentTime = p.timelinePlayers[0]->currentTime();
                p.active[0]->inOutRange = p.timelinePlayers[0]->inOutRange();
                p.active[0]->videoLayer = p.timelinePlayers[0]->videoLayer();
                p.active[0]->volume = p.timelinePlayers[0]->volume();
                p.active[0]->mute = p.timelinePlayers[0]->isMuted();
                p.active[0]->audioOffset = p.timelinePlayers[0]->audioOffset();
            }

            std::vector<qt::TimelinePlayer*> timelinePlayers(items.size(), nullptr);
            auto audioSystem = _context->getSystem<audio::System>();
            for (size_t i = 0; i < items.size(); ++i)
            {
                if (i < p.active.size() && items[i] == p.active[i])
                {
                    timelinePlayers[i] = p.timelinePlayers[i];
                    p.timelinePlayers[i] = nullptr;
                }
                else
                {
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
                        const audio::Info audioInfo = audioSystem->getDefaultOutputInfo();
                        options.ioOptions["ffmpeg/AudioChannelCount"] = string::Format("{0}").arg(audioInfo.channelCount);
                        options.ioOptions["ffmpeg/AudioDataType"] = string::Format("{0}").arg(audioInfo.dataType);
                        options.ioOptions["ffmpeg/AudioSampleRate"] = string::Format("{0}").arg(audioInfo.sampleRate);
                        options.ioOptions["ffmpeg/ThreadCount"] = string::Format("{0}").
                            arg(p.settingsObject->value("Performance/FFmpegThreadCount").toInt());
                        options.pathOptions.maxNumberDigits = std::min(
                            p.settingsObject->value("Misc/MaxFileSequenceDigits").toInt(),
                            255);
                        auto timeline = items[i]->audioPath.isEmpty() ?
                            timeline::Timeline::create(items[i]->path.get(), _context, options) :
                            timeline::Timeline::create(items[i]->path.get(), items[i]->audioPath.get(), _context, options);

                        timeline::PlayerOptions playerOptions;
                        playerOptions.cacheReadAhead = _cacheReadAhead();
                        playerOptions.cacheReadBehind = _cacheReadBehind();
                        playerOptions.timerMode = p.settingsObject->value("Performance/TimerMode").
                            value<timeline::TimerMode>();
                        playerOptions.audioBufferFrameCount = p.settingsObject->value("Performance/AudioBufferFrameCount").
                            value<timeline::AudioBufferFrameCount>();
                        auto timelinePlayer = timeline::TimelinePlayer::create(timeline, _context, playerOptions);

                        qtTimelinePlayer = new qt::TimelinePlayer(timelinePlayer, _context, this);
                    }
                    catch (const std::exception& e)
                    {
                        _log(e.what(), log::Type::Error);
                    }
                    timelinePlayers[i] = qtTimelinePlayer;
                }
            }

            if (!items.empty() &&
                !timelinePlayers.empty() &&
                timelinePlayers[0])
            {
                items[0]->duration = timelinePlayers[0]->duration();
                items[0]->globalStartTime = timelinePlayers[0]->globalStartTime();
                items[0]->ioInfo = timelinePlayers[0]->ioInfo();
                if (!items[0]->init)
                {
                    items[0]->init = true;
                    items[0]->speed = timelinePlayers[0]->speed();
                    items[0]->playback = timelinePlayers[0]->playback();
                    items[0]->loop = timelinePlayers[0]->loop();
                    items[0]->currentTime = timelinePlayers[0]->currentTime();
                    items[0]->inOutRange = timelinePlayers[0]->inOutRange();
                    items[0]->videoLayer = timelinePlayers[0]->videoLayer();
                    items[0]->volume = timelinePlayers[0]->volume();
                    items[0]->mute = timelinePlayers[0]->isMuted();
                    items[0]->audioOffset = timelinePlayers[0]->audioOffset();
                }
                else
                {
                    timelinePlayers[0]->setAudioOffset(items[0]->audioOffset);
                    timelinePlayers[0]->setMute(items[0]->mute);
                    timelinePlayers[0]->setVolume(items[0]->volume);
                    timelinePlayers[0]->setVideoLayer(items[0]->videoLayer);
                    timelinePlayers[0]->setSpeed(items[0]->speed);
                    timelinePlayers[0]->setLoop(items[0]->loop);
                    timelinePlayers[0]->setInOutRange(items[0]->inOutRange);
                    timelinePlayers[0]->seek(items[0]->currentTime);
                    timelinePlayers[0]->setPlayback(items[0]->playback);
                }
            }
            for (size_t i = 1; i < items.size(); ++i)
            {
                if (timelinePlayers[i])
                {
                    timelinePlayers[i]->setVideoLayer(items[i]->videoLayer);
                }
            }

            std::vector<qt::TimelinePlayer*> timelinePlayersValid;
            for (const auto& i : timelinePlayers)
            {
                if (i)
                {
                    if (!timelinePlayersValid.empty())
                    {
                        i->timelinePlayer()->setExternalTime(timelinePlayersValid[0]->timelinePlayer());
                    }
                    timelinePlayersValid.push_back(i);
                }
            }
            if (p.mainWindow)
            {
                p.mainWindow->setTimelinePlayers(timelinePlayersValid);
            }

            p.active = items;
            for (size_t i = 0; i < p.timelinePlayers.size(); ++i)
            {
                delete p.timelinePlayers[i];
            }
            p.timelinePlayers = timelinePlayers;

            _cacheUpdate();
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
            for (const auto& i : p.timelinePlayers)
            {
                if (i)
                {
                    i->setCacheReadAhead(_cacheReadAhead());
                    i->setCacheReadBehind(_cacheReadBehind());
                }
            }
        }
    }
}
