// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2023 Darby Johnston
// All rights reserved.

#include <tlPlayQtApp/App.h>

#include <tlPlayQtApp/DevicesModel.h>
#include <tlPlayQtApp/MainWindow.h>
#include <tlPlayQtApp/MemoryTimeline.h>
#include <tlPlayQtApp/OpenSeparateAudioDialog.h>
#include <tlPlayQtApp/SettingsObject.h>

#include <tlQtWidget/Init.h>
#include <tlQtWidget/FileBrowserSystem.h>
#include <tlQtWidget/Style.h>
#include <tlQtWidget/Util.h>

#include <tlQt/ContextObject.h>
#include <tlQt/MetaTypes.h>
#include <tlQt/OutputDevice.h>
#include <tlQt/TimeObject.h>
#include <tlQt/TimelinePlayer.h>

#include <tlUI/RecentFilesModel.h>

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
#if defined(TLRENDER_USD)
                size_t usdRenderWidth = usd::RenderOptions().renderWidth;
                float usdComplexity = usd::RenderOptions().complexity;
                usd::DrawMode usdDrawMode = usd::RenderOptions().drawMode;
                bool usdEnableLighting = usd::RenderOptions().enableLighting;
                size_t usdStageCache = usd::RenderOptions().stageCacheCount;
                size_t usdDiskCache = usd::RenderOptions().diskCacheByteCount / memory::gigabyte;
#endif // TLRENDER_USD
                std::string logFileName;
                bool resetSettings = false;
            };
        }

        struct App::Private
        {
            Options options;

            QScopedPointer<qt::ContextObject> contextObject;
            std::shared_ptr<file::FileLogSystem> fileLogSystem;
            std::shared_ptr<timeline::TimeUnitsModel> timeUnitsModel;
            QScopedPointer<qt::TimeObject> timeObject;
            QScopedPointer<SettingsObject> settingsObject;
            std::shared_ptr<play::FilesModel> filesModel;
            std::vector<std::shared_ptr<play::FilesModelItem> > files;
            std::vector<std::shared_ptr<play::FilesModelItem> > activeFiles;
            QVector<QSharedPointer<qt::TimelinePlayer> > players;
            std::shared_ptr<ui::RecentFilesModel> recentFilesModel;
            std::shared_ptr<play::ViewportModel> viewportModel;
            std::shared_ptr<play::ColorModel> colorModel;
            QScopedPointer<qt::OutputDevice> outputDevice;
            bool deviceActive = false;
            std::shared_ptr<DevicesModel> devicesModel;
            std::shared_ptr<play::AudioModel> audioModel;

            QScopedPointer<MainWindow> mainWindow;

            std::shared_ptr<observer::ListObserver<std::shared_ptr<play::FilesModelItem> > > filesObserver;
            std::shared_ptr<observer::ListObserver<std::shared_ptr<play::FilesModelItem> > > activeObserver;
            std::shared_ptr<observer::ListObserver<int> > layersObserver;
            std::shared_ptr<observer::ValueObserver<timeline::BackgroundOptions> > backgroundOptionsObserver;
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
            std::string logFileName = play::logFileName(appName, appDocsPath);
            const std::string settingsFileName =
                play::settingsName(appName, appDocsPath);
            IApp::_init(
                app::convert(argc, argv),
                context,
                appName,
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
                    app::CmdLineValueOption<std::string>::create(
                        p.options.logFileName,
                        { "-logFile" },
                        "Log file name.",
                        string::Format("{0}").arg(logFileName)),
                    app::CmdLineFlagOption::create(
                        p.options.resetSettings,
                        { "-resetSettings" },
                        "Reset settings to defaults."),
                });
            const int exitCode = getExit();
            if (exitCode != 0)
            {
                exit(exitCode);
                return;
            }

            // Initialize the file log.
            if (!p.options.logFileName.empty())
            {
                logFileName = p.options.logFileName;
            }
            p.fileLogSystem = file::FileLogSystem::create(logFileName, context);

            // Initialize Qt.
            setOrganizationName("tlRender");
            setApplicationName(QString::fromUtf8(appName.c_str()));
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
            p.contextObject.reset(new qt::ContextObject(context));
            p.timeUnitsModel = timeline::TimeUnitsModel::create(context);
            p.timeObject.reset(new qt::TimeObject(p.timeUnitsModel));

            p.settingsObject.reset(new SettingsObject(p.options.resetSettings, p.timeObject.get()));
            p.settingsObject->setDefaultValue("Cache/Size", 4);
            p.settingsObject->setDefaultValue(
                "Cache/ReadAhead",
                timeline::PlayerCacheOptions().readAhead.value());
            p.settingsObject->setDefaultValue(
                "Cache/ReadBehind",
                timeline::PlayerCacheOptions().readBehind.value());
            p.settingsObject->setDefaultValue(
                "FileSequence/Audio",
                static_cast<int>(timeline::FileSequenceAudio::BaseName));
            p.settingsObject->setDefaultValue("FileSequence/AudioFileName", "");
            p.settingsObject->setDefaultValue("FileSequence/AudioDirectory", "");
            p.settingsObject->setDefaultValue("FileSequence/MaxDigits", 9);
            timeline::BackgroundOptions backgroundOptions;
            p.settingsObject->setDefaultValue("Viewport/Background/Type",
                static_cast<int>(backgroundOptions.type));
            p.settingsObject->setDefaultValue("Viewport/Background/SolidColor",
                qtwidget::toQt(backgroundOptions.solidColor));
            p.settingsObject->setDefaultValue("Viewport/Background/CheckersColor0",
                qtwidget::toQt(backgroundOptions.checkersColor0));
            p.settingsObject->setDefaultValue("Viewport/Background/CheckersColor1",
                qtwidget::toQt(backgroundOptions.checkersColor1));
            p.settingsObject->setDefaultValue("Viewport/Background/CheckersSize",
                qtwidget::toQt(backgroundOptions.checkersSize));
            p.settingsObject->setDefaultValue("Timeline/Editable", false);
            p.settingsObject->setDefaultValue("Timeline/EditAssociatedClips", true);
            p.settingsObject->setDefaultValue("Timeline/FrameView", true);
            p.settingsObject->setDefaultValue("Timeline/StopOnScrub", false);
            p.settingsObject->setDefaultValue("Timeline/Thumbnails", true);
            p.settingsObject->setDefaultValue("Timeline/ThumbnailsSize", 100);
            p.settingsObject->setDefaultValue("Timeline/Transitions", false);
            p.settingsObject->setDefaultValue("Timeline/Markers", false);
            p.settingsObject->setDefaultValue(
                "Performance/TimerMode",
                static_cast<int>(timeline::PlayerOptions().timerMode));
            p.settingsObject->setDefaultValue(
                "Performance/AudioBufferFrameCount",
                static_cast<int>(timeline::PlayerOptions().audioBufferFrameCount));
            p.settingsObject->setDefaultValue("Performance/VideoRequestCount", 16);
            p.settingsObject->setDefaultValue("Performance/AudioRequestCount", 16);
            p.settingsObject->setDefaultValue("Performance/SequenceThreadCount", 16);
            p.settingsObject->setDefaultValue("Performance/FFmpegYUVToRGBConversion", false);
            p.settingsObject->setDefaultValue("Performance/FFmpegThreadCount", 0);
            p.settingsObject->setDefaultValue("Misc/ToolTipsEnabled", true);
            connect(
                p.settingsObject.get(),
                &SettingsObject::valueChanged,
                [this](const QString& name, const QVariant& value)
                {
                    if ("Cache/Size" == name ||
                        "Cache/ReadAhead" == name ||
                        "Cache/ReadBehind" == name)
                    {
                        _cacheUpdate();
                    }
                    else if ("FileBrowser/NativeFileDialog")
                    {
                        if (auto fileBrowserSystem = getContext()->getSystem<qtwidget::FileBrowserSystem>())
                        {
                            fileBrowserSystem->setNativeFileDialog(value.toBool());
                        }
                    }
                });

            p.filesModel = play::FilesModel::create(context);

            p.recentFilesModel = ui::RecentFilesModel::create(context);
            std::vector<file::Path> recent;
            for (const auto& i : p.settingsObject->recentFiles())
            {
                recent.push_back(file::Path(i.toUtf8().data()));
            }
            p.recentFilesModel->setRecent(recent);

            p.viewportModel = play::ViewportModel::create(context);
            backgroundOptions.type = static_cast<timeline::Background>(
                p.settingsObject->value("Viewport/Background/Type").toInt());
            backgroundOptions.solidColor = qtwidget::fromQt(
                p.settingsObject->value("Viewport/Background/SolidColor").value<QColor>());
            backgroundOptions.checkersColor0 = qtwidget::fromQt(
                p.settingsObject->value("Viewport/Background/CheckersColor0").value<QColor>());
            backgroundOptions.checkersColor1 = qtwidget::fromQt(
                p.settingsObject->value("Viewport/Background/CheckersColor1").value<QColor>());
            backgroundOptions.checkersSize = qtwidget::fromQt(
                p.settingsObject->value("Viewport/Background/CheckersSize").toSize());
            p.viewportModel->setBackgroundOptions(backgroundOptions);

            p.colorModel = play::ColorModel::create(context);
            p.colorModel->setColorConfigOptions(p.options.colorConfigOptions);
            p.colorModel->setLUTOptions(p.options.lutOptions);

            p.outputDevice.reset(new qt::OutputDevice(context));
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
            p.settingsObject->setDefaultValue(
                "Devices/HDRMode",
                static_cast<int>(device::HDRMode::FromFile));
            p.devicesModel = DevicesModel::create(context);
            p.devicesModel->setDeviceIndex(
                p.settingsObject->value("Devices/DeviceIndex").toInt());
            p.devicesModel->setDisplayModeIndex(
                p.settingsObject->value("Devices/DisplayModeIndex").toInt());
            p.devicesModel->setPixelTypeIndex(
                p.settingsObject->value("Devices/PixelTypeIndex").toInt());
            p.devicesModel->setDeviceEnabled(
                p.settingsObject->value("Devices/DeviceEnabled").toBool());
            p.devicesModel->setHDRMode(static_cast<device::HDRMode>(
                p.settingsObject->value("Devices/HDRMode").toInt()));
            std::string s = p.settingsObject->value("Devices/HDRData").toString().toUtf8().data();
            if (!s.empty())
            {
                auto json = nlohmann::json::parse(s);
                image::HDRData hdrData;
                try
                {
                    from_json(json, hdrData);
                }
                catch (const std::exception&)
                {}
                p.devicesModel->setHDRData(hdrData);
            }

            p.settingsObject->setDefaultValue("FileBrowser/NativeFileDialog", true);
            if (auto fileBrowserSystem = _context->getSystem<qtwidget::FileBrowserSystem>())
            {
                fileBrowserSystem->setNativeFileDialog(
                    p.settingsObject->value("FileBrowser/NativeFileDialog").toBool());
                fileBrowserSystem->setPath(
                    p.settingsObject->value("FileBrowser/Path").toString().toUtf8().data());
            }

            p.audioModel = play::AudioModel::create(context);

            // Create observers.
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

            p.backgroundOptionsObserver = observer::ValueObserver<timeline::BackgroundOptions>::create(
                p.viewportModel->observeBackgroundOptions(),
                [this](const timeline::BackgroundOptions& value)
                {
                    _p->settingsObject->setValue("Viewport/Background/Type",
                        static_cast<int>(value.type));
                    _p->settingsObject->setValue("Viewport/Background/SolidColor",
                        qtwidget::toQt(value.solidColor));
                    _p->settingsObject->setValue("Viewport/Background/CheckersColor0",
                        qtwidget::toQt(value.checkersColor0));
                    _p->settingsObject->setValue("Viewport/Background/CheckersColor1",
                        qtwidget::toQt(value.checkersColor1));
                    _p->settingsObject->setValue("Viewport/Background/CheckersSize",
                        qtwidget::toQt(value.checkersSize));
                });

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

            // Create the main window.
            p.mainWindow.reset(new MainWindow(this));

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
            p.mainWindow.reset();
            if (p.settingsObject)
            {
                QList<QString> recent;
                for (const auto& i : p.recentFilesModel->getRecent())
                {
                    recent.push_back(QString::fromUtf8(i.get().c_str()));
                }
                p.settingsObject->setRecentFiles(recent);

                if (auto fileBrowserSystem = _context->getSystem<qtwidget::FileBrowserSystem>())
                {
                    p.settingsObject->setValue(
                        "FileBrowser/Path",
                        QString::fromUtf8(fileBrowserSystem->getPath().c_str()));
                }

                if (p.devicesModel)
                {
                    const auto& deviceData = p.devicesModel->observeData()->get();
                    p.settingsObject->setValue(
                        "Devices/DeviceIndex",
                        deviceData.deviceIndex);
                    p.settingsObject->setValue(
                        "Devices/DisplayModeIndex",
                        deviceData.displayModeIndex);
                    p.settingsObject->setValue(
                        "Devices/PixelTypeIndex",
                        deviceData.pixelTypeIndex);
                    p.settingsObject->setValue(
                        "Devices/DeviceEnabled",
                        deviceData.deviceEnabled);
                    p.settingsObject->setValue(
                        "Devices/HDRMode",
                        static_cast<int>(deviceData.hdrMode));
                    nlohmann::json json;
                    to_json(json, deviceData.hdrData);
                    p.settingsObject->setValue(
                        "Devices/HDRData",
                        QString::fromUtf8(json.dump().c_str()));
                }
            }
        }

        const std::shared_ptr<timeline::TimeUnitsModel>& App::timeUnitsModel() const
        {
            return _p->timeUnitsModel;
        }

        qt::TimeObject* App::timeObject() const
        {
            return _p->timeObject.get();
        }

        SettingsObject* App::settingsObject() const
        {
            return _p->settingsObject.get();
        }

        const std::shared_ptr<play::FilesModel>& App::filesModel() const
        {
            return _p->filesModel;
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

        void App::open(const QString& fileName, const QString& audioFileName)
        {
            TLRENDER_P();
            file::PathOptions pathOptions;
            pathOptions.maxNumberDigits = p.settingsObject->value(
                "FileSequence/MaxDigits").toInt();
            for (const auto& path : timeline::getPaths(
                file::Path(fileName.toUtf8().data()),
                pathOptions,
                _context))
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
                        options.fileSequenceAudio = p.settingsObject->value(
                            "FileSequence/Audio").value<timeline::FileSequenceAudio>();
                        options.fileSequenceAudioFileName = p.settingsObject->value(
                            "FileSequence/AudioFileName").toString().toUtf8().data();
                        options.fileSequenceAudioDirectory = p.settingsObject->value(
                            "FileSequence/AudioDirectory").toString().toUtf8().data();
                        options.videoRequestCount = p.settingsObject->value(
                            "Performance/VideoRequestCount").toInt();
                        options.audioRequestCount = p.settingsObject->value(
                            "Performance/AudioRequestCount").toInt();
                        options.ioOptions["SequenceIO/ThreadCount"] = string::Format("{0}").
                            arg(p.settingsObject->value("Performance/SequenceThreadCount").toInt());
                        options.ioOptions["FFmpeg/YUVToRGBConversion"] = string::Format("{0}").
                            arg(p.settingsObject->value("Performance/FFmpegYUVToRGBConversion").toBool());
                        options.ioOptions["FFmpeg/ThreadCount"] = string::Format("{0}").
                            arg(p.settingsObject->value("Performance/FFmpegThreadCount").toInt());
                        options.pathOptions.maxNumberDigits =
                            p.settingsObject->value("FileSequence/MaxDigits").toInt();
                        auto otioTimeline = items[i]->audioPath.isEmpty() ?
                            timeline::create(items[i]->path, _context, options) :
                            timeline::create(items[i]->path, items[i]->audioPath, _context, options);
                        if (0)
                        {
                            createMemoryTimeline(
                                otioTimeline,
                                items[i]->path.getDirectory(),
                                options.pathOptions);
                        }
                        auto timeline = timeline::Timeline::create(otioTimeline, _context, options);
                        const otime::TimeRange& timeRange = timeline->getTimeRange();

                        timeline::PlayerOptions playerOptions;
                        playerOptions.cache.readAhead = time::invalidTime;
                        playerOptions.cache.readBehind = time::invalidTime;
                        playerOptions.timerMode = p.settingsObject->value(
                            "Performance/TimerMode").value<timeline::TimerMode>();
                        playerOptions.audioBufferFrameCount = p.settingsObject->value(
                            "Performance/AudioBufferFrameCount").toInt();
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

            auto activePlayers = _activePlayers();
            if (!activePlayers.empty() && activePlayers[0])
            {
                activePlayers[0]->setPlayback(timeline::Playback::Stop);
            }

            p.activeFiles = items;
            activePlayers = _activePlayers();
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
            p.outputDevice->setTimelinePlayers(activePlayers);

            _cacheUpdate();
            _audioUpdate();

            Q_EMIT activePlayersChanged(activePlayers);
        }

        QVector<QSharedPointer<qt::TimelinePlayer> > App::_activePlayers() const
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

            // Update the I/O cache.
            auto ioSystem = _context->getSystem<io::System>();
            ioSystem->getCache()->setMax(
                p.settingsObject->value("Cache/Size").toInt() * memory::gigabyte);

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
                    player->setMute(mute || p.deviceActive);
                }
            }
            if (p.outputDevice)
            {
                p.outputDevice->setVolume(volume);
                p.outputDevice->setMute(mute);
                const auto activePlayers = _activePlayers();
                p.outputDevice->setAudioOffset(
                    (!activePlayers.empty() && activePlayers[0]) ?
                    activePlayers[0]->audioOffset() :
                    0.0);
            }
        }
    }
}
