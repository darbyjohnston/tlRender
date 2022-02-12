// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2022 Darby Johnston
// All rights reserved.

#include <tlPlay/App.h>

#include <tlPlay/OpenWithAudioDialog.h>

#include <tlQWidget/Util.h>

#include <tlCore/AudioSystem.h>
#include <tlCore/Math.h>
#include <tlCore/StringFormat.h>
#include <tlCore/Time.h>
#include <tlCore/TimelineUtil.h>

#include <QFileDialog>

namespace tl
{
    namespace play
    {
        App::App(int& argc, char** argv) :
            QApplication(argc, argv)
        {
            IApp::_init(
                argc,
                argv,
                "tlplay",
                "Play timelines, movies, and image sequences.",
                {
                    app::CmdLineValueArg<std::string>::create(
                        _input,
                        "input",
                        "The input timeline.",
                        true)
                },
            {
                app::CmdLineValueOption<std::string>::create(
                    _options.colorConfig.fileName,
                    { "-colorConfig", "-cc" },
                    "Color configuration file (config.ocio)."),
                app::CmdLineValueOption<std::string>::create(
                    _options.colorConfig.input,
                    { "-colorInput", "-ci" },
                    "Input color space."),
                app::CmdLineValueOption<std::string>::create(
                    _options.colorConfig.display,
                    { "-colorDisplay", "-cd" },
                    "Display color space."),
                app::CmdLineValueOption<std::string>::create(
                    _options.colorConfig.view,
                    { "-colorView", "-cv" },
                    "View color space.")
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

            // Create objects.
            _timeObject = new qt::TimeObject(this);

            _settingsObject = new SettingsObject(_timeObject, this);
            connect(
                _settingsObject,
                SIGNAL(cacheReadAheadChanged(double)),
                SLOT(_settingsCallback()));
            connect(
                _settingsObject,
                SIGNAL(cacheReadBehindChanged(double)),
                SLOT(_settingsCallback()));
            connect(
                _settingsObject,
                SIGNAL(videoRequestCountChanged(int)),
                SLOT(_settingsCallback()));
            connect(
                _settingsObject,
                SIGNAL(audioRequestCountChanged(int)),
                SLOT(_settingsCallback()));
            connect(
                _settingsObject,
                SIGNAL(sequenceThreadCountChanged(int)),
                SLOT(_settingsCallback()));
            connect(
                _settingsObject,
                SIGNAL(ffmpegThreadCountChanged(int)),
                SLOT(_settingsCallback()));
            _settingsUpdate();

            _filesModel = FilesModel::create(_context);
            _activeObserver = observer::ListObserver<std::shared_ptr<FilesModelItem> >::create(
                _filesModel->observeActive(),
                [this](const std::vector<std::shared_ptr<FilesModelItem> >& value)
                {
                    _activeCallback(value);
                });
            _layersObserver = observer::ListObserver<int>::create(
                _filesModel->observeLayers(),
                [this](const std::vector<int>& value)
                {
                    for (size_t i = 0; i < value.size() && i < _timelinePlayers.size(); ++i)
                    {
                        if (_timelinePlayers[i])
                        {
                            _timelinePlayers[i]->setVideoLayer(value[i]);
                        }
                    }
                });

            _colorModel = ColorModel::create(_context);
            if (!_options.colorConfig.fileName.empty())
            {
                _colorModel->setConfig(_options.colorConfig);
            }

            // Set the dark style color palette.
            setPalette(qwidget::darkStyle());

            // Create the main window.
            _mainWindow = new MainWindow(this);

            // Open the input file.
            if (!_input.empty())
            {
                open(QString::fromUtf8(_input.c_str()));
            }

            _mainWindow->show();
        }

        App::~App()
        {
            delete _mainWindow;
            //! \bug Why is it necessary to manually delete this to get the settings to save?
            delete _settingsObject;
        }

        qt::TimeObject* App::timeObject() const
        {
            return _timeObject;
        }

        SettingsObject* App::settingsObject() const
        {
            return _settingsObject;
        }

        const std::shared_ptr<FilesModel>& App::filesModel() const
        {
            return _filesModel;
        }

        const std::shared_ptr<ColorModel>& App::colorModel() const
        {
            return _colorModel;
        }

        void App::open(const QString& fileName, const QString& audioFileName)
        {
            auto item = std::make_shared<FilesModelItem>();
            item->path = file::Path(fileName.toUtf8().data());
            item->audioPath = file::Path(audioFileName.toUtf8().data());
            _filesModel->add(item);
            _settingsObject->addRecentFile(fileName);
        }

        void App::openDialog()
        {
            std::vector<std::string> extensions;
            for (const auto& i : timeline::getExtensions(
                static_cast<int>(avio::FileExtensionType::VideoAndAudio) |
                static_cast<int>(avio::FileExtensionType::VideoOnly) |
                static_cast<int>(avio::FileExtensionType::AudioOnly),
                _context))
            {
                extensions.push_back("*" + i);
            }

            QString dir;
            if (!_active.empty())
            {
                dir = QString::fromUtf8(_active[0]->path.get().c_str());
            }

            const auto fileName = QFileDialog::getOpenFileName(
                _mainWindow,
                tr("Open"),
                dir,
                tr("Files") + " (" + QString::fromUtf8(string::join(extensions, " ").c_str()) + ")");
            if (!fileName.isEmpty())
            {
                open(fileName);
            }
        }

        void App::openWithAudioDialog()
        {
            auto dialog = std::make_unique<OpenWithAudioDialog>(_context);
            if (QDialog::Accepted == dialog->exec())
            {
                open(dialog->videoFileName(), dialog->audioFileName());
            }
        }

        void App::_activeCallback(const std::vector<std::shared_ptr<FilesModelItem> >& items)
        {
            if (!_active.empty() &&
                !_timelinePlayers.empty() &&
                _timelinePlayers[0])
            {
                _active[0]->init = true;
                _active[0]->speed = _timelinePlayers[0]->speed();
                _active[0]->playback = _timelinePlayers[0]->playback();
                _active[0]->loop = _timelinePlayers[0]->loop();
                _active[0]->currentTime = _timelinePlayers[0]->currentTime();
                _active[0]->inOutRange = _timelinePlayers[0]->inOutRange();
                _active[0]->videoLayer = _timelinePlayers[0]->videoLayer();
                _active[0]->volume = _timelinePlayers[0]->volume();
                _active[0]->mute = _timelinePlayers[0]->isMuted();
                _active[0]->audioOffset = _timelinePlayers[0]->audioOffset();
            }

            std::vector<qt::TimelinePlayer*> timelinePlayers(items.size(), nullptr);
            for (size_t i = 0; i < items.size(); ++i)
            {
                if (i < items.size() &&
                    i < _active.size() &&
                    items[i] == _active[i])
                {
                    timelinePlayers[i] = _timelinePlayers[i];
                    _timelinePlayers[i] = nullptr;
                }
                else
                {
                    qt::TimelinePlayer* qtTimelinePlayer = nullptr;
                    try
                    {
                        timeline::Options options;
                        options.fileSequenceAudio = _settingsObject->fileSequenceAudio();
                        options.fileSequenceAudioFileName = _settingsObject->fileSequenceAudioFileName().toUtf8().data();
                        options.fileSequenceAudioDirectory = _settingsObject->fileSequenceAudioDirectory().toUtf8().data();
                        options.videoRequestCount = _settingsObject->videoRequestCount();
                        options.audioRequestCount = _settingsObject->audioRequestCount();
                        options.avioOptions["SequenceIO/ThreadCount"] = string::Format("{0}").arg(_settingsObject->sequenceThreadCount());
                        auto audioSystem = _context->getSystem<audio::System>();
                        const audio::Info audioInfo = audioSystem->getDefaultOutputInfo();
                        options.avioOptions["ffmpeg/AudioChannelCount"] = string::Format("{0}").arg(audioInfo.channelCount);
                        options.avioOptions["ffmpeg/AudioDataType"] = string::Format("{0}").arg(audioInfo.dataType);
                        options.avioOptions["ffmpeg/AudioSampleRate"] = string::Format("{0}").arg(audioInfo.sampleRate);
                        options.avioOptions["ffmpeg/ThreadCount"] = string::Format("{0}").arg(_settingsObject->ffmpegThreadCount());
                        options.pathOptions.maxNumberDigits = std::min(_settingsObject->maxFileSequenceDigits(), 255);
                        auto timeline = items[i]->audioPath.isEmpty() ?
                            timeline::Timeline::create(items[i]->path.get(), _context, options) :
                            timeline::Timeline::create(items[i]->path.get(), items[i]->audioPath.get(), _context, options);

                        timeline::PlayerOptions playerOptions;
                        playerOptions.timerMode = _settingsObject->timerMode();
                        playerOptions.audioBufferFrameCount = _settingsObject->audioBufferFrameCount();
                        auto timelinePlayer = timeline::TimelinePlayer::create(timeline, _context, playerOptions);
                        timelinePlayer->setCacheReadAhead(otime::RationalTime(_settingsObject->cacheReadAhead(), 1.0));
                        timelinePlayer->setCacheReadBehind(otime::RationalTime(_settingsObject->cacheReadBehind(), 1.0));

                        qtTimelinePlayer = new qt::TimelinePlayer(timelinePlayer, _context, this);
                    }
                    catch (const std::exception& e)
                    {
                        _log(e.what(), core::LogType::Error);
                    }
                    timelinePlayers[i] = qtTimelinePlayer;
                }
            }

            if (!items.empty() &&
                !timelinePlayers.empty() &&
                timelinePlayers[0])
            {
                if (!items[0]->init)
                {
                    items[0]->init = true;
                    items[0]->duration = timelinePlayers[0]->duration();
                    items[0]->globalStartTime = timelinePlayers[0]->globalStartTime();
                    items[0]->avInfo = timelinePlayers[0]->avInfo();
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
            if (_mainWindow)
            {
                _mainWindow->setTimelinePlayers(timelinePlayersValid);
            }

            _active = items;
            for (size_t i = 0; i < _timelinePlayers.size(); ++i)
            {
                delete _timelinePlayers[i];
            }
            _timelinePlayers = timelinePlayers;
        }

        void App::_settingsCallback()
        {
            _settingsUpdate();
        }

        void App::_settingsUpdate()
        {
            for (const auto& i : _timelinePlayers)
            {
                i->setCacheReadAhead(otime::RationalTime(_settingsObject->cacheReadAhead(), 1.0));
                i->setCacheReadBehind(otime::RationalTime(_settingsObject->cacheReadBehind(), 1.0));
            }
        }
    }
}
