// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2022 Darby Johnston
// All rights reserved.

#include "App.h"

#include "OpenWithAudioDialog.h"

#include <tlrCore/AudioSystem.h>
#include <tlrCore/Math.h>
#include <tlrCore/StringFormat.h>
#include <tlrCore/Time.h>
#include <tlrCore/TimelineUtil.h>

#include <QFileDialog>
#include <QMessageBox>

namespace tlr
{
    App::App(int& argc, char** argv) :
        QApplication(argc, argv)
    {
        IApp::_init(
            argc,
            argv,
            "play-qwidget",
            "Play an editorial timeline.",
            {
                app::CmdLineValueArg<std::string>::create(
                    _input,
                    "input",
                    "The input timeline.",
                    true)
            },
            {
                app::CmdLineValueOption<std::string>::create(
                    _options.colorConfig.config,
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
        QCoreApplication::setApplicationName("play-qwidget");
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

        _filesModel = new FilesModel(this);
        connect(
            _filesModel,
            SIGNAL(activeChanged(const std::vector<std::shared_ptr<FilesModelItem> >&)),
            SLOT(_activeCallback(const std::vector<std::shared_ptr<FilesModelItem> >&)));
        connect(
            _filesModel,
            SIGNAL(layerChanged(const std::shared_ptr<FilesModelItem>&, int)),
            SLOT(_layerCallback(const std::shared_ptr<FilesModelItem>&, int)));

        // Create the main window.
        _mainWindow = new MainWindow(this);
        _mainWindow->setColorConfig(_options.colorConfig);

        // Open the input file.
        if (!_input.empty())
        {
            open(QString::fromUtf8(_input.c_str()));
        }

        _mainWindow->show();
    }

    App::~App()
    {
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

    FilesModel* App::filesModel() const
    {
        return _filesModel;
    }

    void App::open(const QString& fileName, const QString& audioFileName)
    {
        auto item = std::make_shared<FilesModelItem>();
        item->path = file::Path(fileName.toUtf8().data());
        item->audioPath = file::Path(audioFileName.toUtf8().data());
        _filesModel->add(item);
        _settingsObject->addRecentFile(fileName);
    }

    void App::open()
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
        if (!_activeItems.empty())
        {
            dir = QString::fromUtf8(_activeItems[0]->path.get().c_str());
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

    void App::openWithAudio()
    {
        auto dialog = std::make_unique<OpenWithAudioDialog>(_context);
        if (QDialog::Accepted == dialog->exec())
        {
            open(dialog->videoFileName(), dialog->audioFileName());
        }
    }

    void App::_activeCallback(const std::vector<std::shared_ptr<FilesModelItem> >& items)
    {
        for (size_t i = 0; i < _activeItems.size(); ++i)
        {
            _activeItems[i]->init = true;
            _activeItems[i]->speed = _timelinePlayers[i]->speed();
            _activeItems[i]->playback = _timelinePlayers[i]->playback();
            _activeItems[i]->loop = _timelinePlayers[i]->loop();
            _activeItems[i]->currentTime = _timelinePlayers[i]->currentTime();
            _activeItems[i]->inOutRange = _timelinePlayers[i]->inOutRange();
            _activeItems[i]->videoLayer = _timelinePlayers[i]->videoLayer();
            _activeItems[i]->volume = _timelinePlayers[i]->volume();
            _activeItems[i]->mute = _timelinePlayers[i]->isMuted();
            _activeItems[i]->audioOffset = _timelinePlayers[i]->audioOffset();
        }

        _activeItems = items;

        std::vector<qt::TimelinePlayer*> qtTimelinePlayers;
        for (const auto& i : _activeItems)
        {
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
                auto timeline = i->audioPath.isEmpty() ?
                    timeline::Timeline::create(i->path.get(), _context, options) :
                    timeline::Timeline::create(i->path.get(), i->audioPath.get(), _context, options);

                timeline::PlayerOptions playerOptions;
                playerOptions.timerMode = _settingsObject->timerMode();
                playerOptions.audioBufferFrameCount = _settingsObject->audioBufferFrameCount();
                auto timelinePlayer = timeline::TimelinePlayer::create(timeline, _context, playerOptions);

                if (!i->init)
                {
                    i->init = true;
                    i->duration = timelinePlayer->getDuration();
                    i->globalStartTime = timelinePlayer->getGlobalStartTime();
                    i->avInfo = timelinePlayer->getAVInfo();
                    i->speed = timelinePlayer->observeSpeed()->get();
                    i->playback = timelinePlayer->observePlayback()->get();
                    i->loop = timelinePlayer->observeLoop()->get();
                    i->currentTime = timelinePlayer->observeCurrentTime()->get();
                    i->inOutRange = timelinePlayer->observeInOutRange()->get();
                    i->videoLayer = timelinePlayer->observeVideoLayer()->get();
                    i->volume = timelinePlayer->observeVolume()->get();
                    i->mute = timelinePlayer->observeMute()->get();
                    i->audioOffset = timelinePlayer->observeAudioOffset()->get();
                }
                else
                {
                    timelinePlayer->setAudioOffset(i->audioOffset);
                    timelinePlayer->setMute(i->mute);
                    timelinePlayer->setVolume(i->volume);
                    timelinePlayer->setVideoLayer(i->videoLayer);
                    timelinePlayer->setSpeed(i->speed);
                    timelinePlayer->setLoop(i->loop);
                    timelinePlayer->setInOutRange(i->inOutRange);
                    timelinePlayer->seek(i->currentTime);
                    timelinePlayer->setPlayback(i->playback);
                }

                qtTimelinePlayers.push_back(new qt::TimelinePlayer(timelinePlayer, _context, this));
            }
            catch (const std::exception& e)
            {
                QMessageBox dialog;
                dialog.setText(e.what());
                dialog.exec();
            }
        }

        _mainWindow->setTimelinePlayers(qtTimelinePlayers);

        for (size_t i = 0; i < _timelinePlayers.size(); ++i)
        {
            delete _timelinePlayers[i];
        }
        _timelinePlayers = qtTimelinePlayers;

        _settingsUpdate();
    }

    void App::_layerCallback(const std::shared_ptr<FilesModelItem>& item, int layer)
    {
        const auto i = std::find(_activeItems.begin(), _activeItems.end(), item);
        if (i != _activeItems.end())
        {
            const size_t index = i - _activeItems.begin();
            _timelinePlayers[index]->setVideoLayer(layer);
        }
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
