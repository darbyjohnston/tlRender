// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2022 Darby Johnston
// All rights reserved.

#include "App.h"

#include <tlrCore/AudioSystem.h>
#include <tlrCore/Math.h>
#include <tlrCore/StringFormat.h>
#include <tlrCore/Time.h>
#include <tlrCore/TimelineUtil.h>

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
        _filesModel = new FilesModel(_context, this);
        connect(
            _filesModel,
            SIGNAL(currentChanged(const FilesModelItem*)),
            SLOT(_filesModelCallback(const FilesModelItem*)));

        // Create the main window.
        _mainWindow = new MainWindow(_settingsObject, _timeObject, _context);
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

    FilesModel* App::filesModel() const
    {
        return _filesModel;
    }

    void App::open(const QString& fileName)
    {
        _filesModel->add(FilesModelItem(fileName.toUtf8().data()));
        _settingsObject->addRecentFile(fileName);
    }

    void App::openWithAudio(const QString& fileName, const QString& audioFileName)
    {
        _filesModel->add(FilesModelItem(fileName.toUtf8().data(), audioFileName.toUtf8().data()));
        _settingsObject->addRecentFile(fileName);
    }

    void App::_filesModelCallback(const FilesModelItem* item)
    {
        qt::TimelinePlayer* qtTimelinePlayer = nullptr;
        if (item)
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
                auto timeline = item->audioPath.isEmpty() ?
                    timeline::Timeline::create(item->path.get(), _context, options) :
                    timeline::Timeline::create(item->path.get(), item->audioPath.get(), _context, options);

                timeline::PlayerOptions playerOptions;
                playerOptions.timerMode = _settingsObject->timerMode();
                playerOptions.audioBufferFrameCount = _settingsObject->audioBufferFrameCount();
                auto timelinePlayer = timeline::TimelinePlayer::create(timeline, _context, playerOptions);
                if (item->init)
                {
                    timelinePlayer->setAudioOffset(item->audioOffset);
                    timelinePlayer->setMute(item->mute);
                    timelinePlayer->setVolume(item->volume);
                    timelinePlayer->setVideoLayer(item->videoLayer);
                    timelinePlayer->setSpeed(item->speed);
                    timelinePlayer->setLoop(item->loop);
                    timelinePlayer->setInOutRange(item->inOutRange);
                    timelinePlayer->seek(item->currentTime);
                    timelinePlayer->setPlayback(item->playback);
                }
                else
                {
                    _filesModel->update(timelinePlayer);
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

        _mainWindow->setTimelinePlayer(qtTimelinePlayer);

        delete _timelinePlayer;
        _timelinePlayer = qtTimelinePlayer;

        if (_timelinePlayer)
        {
            connect(
                _timelinePlayer,
                SIGNAL(speedChanged(double)),
                SLOT(_timelinePlayerCallback()));
            connect(
                _timelinePlayer,
                SIGNAL(playbackChanged(tlr::timeline::Playback)),
                SLOT(_timelinePlayerCallback()));
            connect(
                _timelinePlayer,
                SIGNAL(loopChanged(tlr::timeline::Loop)),
                SLOT(_timelinePlayerCallback()));
            connect(
                _timelinePlayer,
                SIGNAL(currentTimeChanged(const otime::RationalTime&)),
                SLOT(_timelinePlayerCallback()));
            connect(
                _timelinePlayer,
                SIGNAL(inOutRangeChanged(const otime::TimeRange&)),
                SLOT(_timelinePlayerCallback()));
            connect(
                _timelinePlayer,
                SIGNAL(videoLayerChanged(int)),
                SLOT(_timelinePlayerCallback()));
            connect(
                _timelinePlayer,
                SIGNAL(volumeChanged(float)),
                SLOT(_timelinePlayerCallback()));
            connect(
                _timelinePlayer,
                SIGNAL(muteChanged(bool)),
                SLOT(_timelinePlayerCallback()));
            connect(
                _timelinePlayer,
                SIGNAL(audioOffsetChanged(double)),
                SLOT(_timelinePlayerCallback()));
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

    void App::_timelinePlayerCallback()
    {
        _filesModel->update(_timelinePlayer->timelinePlayer());
    }
}
