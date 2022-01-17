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
            SIGNAL(currentChanged(const std::shared_ptr<FilesModelItem>&)),
            SLOT(_filesModelCallback(const std::shared_ptr<FilesModelItem>&)));
        
        _layersModel = new LayersModel(this);
        connect(
            _layersModel,
            SIGNAL(currentChanged(int)),
            SLOT(_layersModelCallback(int)));

        // Create the main window.
        _mainWindow = new MainWindow(_filesModel, _layersModel, _settingsObject, _timeObject, _context);
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

    void App::open(const QString& fileName)
    {
        _filesModel->add(fileName.toUtf8().data());
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
        if (const auto item = _filesModel->current())
        {
            dir = QString::fromUtf8(item->path.get().c_str());
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

    void App::openWithAudio(const QString& fileName, const QString& audioFileName)
    {
        _filesModel->add(fileName.toUtf8().data(), audioFileName.toUtf8().data());
        _settingsObject->addRecentFile(fileName);
    }

    void App::openWithAudio()
    {
        auto dialog = std::make_unique<OpenWithAudioDialog>(_context);
        if (QDialog::Accepted == dialog->exec())
        {
            openWithAudio(dialog->videoFileName(), dialog->audioFileName());
        }
    }

    void App::close()
    {
        _filesModel->remove();
    }

    void App::closeAll()
    {
        _filesModel->clear();
    }

    void App::_filesModelCallback(const std::shared_ptr<FilesModelItem>& item)
    {
        if (_currentFile)
        {
            _currentFile->init = true;
            _currentFile->duration = _timelinePlayer->duration();
            _currentFile->globalStartTime = _timelinePlayer->globalStartTime();
            _currentFile->avInfo = _timelinePlayer->avInfo();
            _currentFile->speed = _timelinePlayer->speed();
            _currentFile->playback = _timelinePlayer->playback();
            _currentFile->loop = _timelinePlayer->loop();
            _currentFile->currentTime = _timelinePlayer->currentTime();
            _currentFile->inOutRange = _timelinePlayer->inOutRange();
            _currentFile->videoLayer = _timelinePlayer->videoLayer();
            _currentFile->volume = _timelinePlayer->volume();
            _currentFile->mute = _timelinePlayer->isMuted();
            _currentFile->audioOffset = _timelinePlayer->audioOffset();
        }

        _currentFile = item;

        std::shared_ptr<timeline::Timeline> timeline;
        qt::TimelinePlayer* qtTimelinePlayer = nullptr;
        if (_currentFile)
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
                timeline = _currentFile->audioPath.isEmpty() ?
                    timeline::Timeline::create(_currentFile->path.get(), _context, options) :
                    timeline::Timeline::create(_currentFile->path.get(), _currentFile->audioPath.get(), _context, options);

                timeline::PlayerOptions playerOptions;
                playerOptions.timerMode = _settingsObject->timerMode();
                playerOptions.audioBufferFrameCount = _settingsObject->audioBufferFrameCount();
                auto timelinePlayer = timeline::TimelinePlayer::create(timeline, _context, playerOptions);

                if (!_currentFile->init)
                {
                    _currentFile->init = true;
                    _currentFile->duration = timelinePlayer->getDuration();
                    _currentFile->globalStartTime = timelinePlayer->getGlobalStartTime();
                    _currentFile->avInfo = timelinePlayer->getAVInfo();
                    _currentFile->speed = timelinePlayer->observeSpeed()->get();
                    _currentFile->playback = timelinePlayer->observePlayback()->get();
                    _currentFile->loop = timelinePlayer->observeLoop()->get();
                    _currentFile->currentTime = timelinePlayer->observeCurrentTime()->get();
                    _currentFile->inOutRange = timelinePlayer->observeInOutRange()->get();
                    _currentFile->videoLayer = timelinePlayer->observeVideoLayer()->get();
                    _currentFile->volume = timelinePlayer->observeVolume()->get();
                    _currentFile->mute = timelinePlayer->observeMute()->get();
                    _currentFile->audioOffset = timelinePlayer->observeAudioOffset()->get();
                }
                else
                {
                    timelinePlayer->setAudioOffset(_currentFile->audioOffset);
                    timelinePlayer->setMute(_currentFile->mute);
                    timelinePlayer->setVolume(_currentFile->volume);
                    timelinePlayer->setVideoLayer(_currentFile->videoLayer);
                    timelinePlayer->setSpeed(_currentFile->speed);
                    timelinePlayer->setLoop(_currentFile->loop);
                    timelinePlayer->setInOutRange(_currentFile->inOutRange);
                    timelinePlayer->seek(_currentFile->currentTime);
                    timelinePlayer->setPlayback(_currentFile->playback);
                }

                qtTimelinePlayer = new qt::TimelinePlayer(timelinePlayer, _context, this);
            }
            catch (const std::exception& e)
            {
                QMessageBox dialog;
                dialog.setText(e.what());
                dialog.exec();
            }
        }

        _layersModel->set(
            _currentFile ? _currentFile->avInfo.video : std::vector<imaging::Info>(),
            _currentFile ? _currentFile->videoLayer : -1);

        _mainWindow->setTimelinePlayer(qtTimelinePlayer);

        delete _timelinePlayer;
        _timelinePlayer = qtTimelinePlayer;

        _settingsUpdate();
    }

    void App::_layersModelCallback(int value)
    {
        if (_timelinePlayer)
        {
            _timelinePlayer->setVideoLayer(value);
        }
    }

    void App::_settingsCallback()
    {
        _settingsUpdate();
    }

    void App::_settingsUpdate()
    {
        if (_timelinePlayer)
        {
            _timelinePlayer->setCacheReadAhead(otime::RationalTime(_settingsObject->cacheReadAhead(), 1.0));
            _timelinePlayer->setCacheReadBehind(otime::RationalTime(_settingsObject->cacheReadBehind(), 1.0));
        }
    }
}
