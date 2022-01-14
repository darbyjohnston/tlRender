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
        _timelineListModel = new TimelineListModel(_context, this);

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

    TimelineListModel* App::timelineListModel() const
    {
        return _timelineListModel;
    }

    int App::current() const
    {
        return _current;
    }

    void App::open(const QString& fileName)
    {
        _current = _timelineListModel->rowCount();
        auto timelinePlayer = _createTimelinePlayer(fileName.toUtf8().data());
        _timelineListModel->add(timelinePlayer ? timelinePlayer->timelinePlayer() : TimelineListItem());
        _mainWindow->setTimelinePlayer(timelinePlayer);
        if (_timelinePlayer)
        {
            delete _timelinePlayer;
            _timelinePlayer = nullptr;
        }
        _timelinePlayer = timelinePlayer;
        _settingsObject->addRecentFile(fileName);
    }

    void App::openWithAudio(const QString& fileName, const QString& audioFileName)
    {
        _current = _timelineListModel->rowCount();
        auto timelinePlayer = _createTimelinePlayer(fileName.toUtf8().data(), audioFileName.toUtf8().data());
        _timelineListModel->add(timelinePlayer ? timelinePlayer->timelinePlayer() : TimelineListItem());
        _mainWindow->setTimelinePlayer(timelinePlayer);
        if (_timelinePlayer)
        {
            delete _timelinePlayer;
            _timelinePlayer = nullptr;
        }
        _timelinePlayer = timelinePlayer;
        _settingsObject->addRecentFile(fileName);
    }

    void App::close()
    {
        if (_current >= 0 && _current < _timelineListModel->rowCount())
        {
            _timelineListModel->remove(_current);
            _current = std::min(_current, _timelineListModel->rowCount() - 1);
            qt::TimelinePlayer* timelinePlayer = _current != -1 ?
                _createTimelinePlayer(_timelineListModel->get(_current)) :
                nullptr;
            _mainWindow->setTimelinePlayer(timelinePlayer);
            delete _timelinePlayer;
            _timelinePlayer = timelinePlayer;
        }
    }

    void App::closeAll()
    {
        while (_timelineListModel->rowCount() > 0)
        {
            _timelineListModel->remove(0);
        }
        _current = -1;
        _mainWindow->setTimelinePlayer(nullptr);
        delete _timelinePlayer;
        _timelinePlayer = nullptr;
    }

    void App::setCurrent(int value)
    {
        if (value == _current)
            return;
        _current = value;
        qt::TimelinePlayer* timelinePlayer = _current != -1 ?
            _createTimelinePlayer(_timelineListModel->get(_current)) :
            nullptr;
        _mainWindow->setTimelinePlayer(timelinePlayer);
        delete _timelinePlayer;
        _timelinePlayer = timelinePlayer;
    }

    void App::_timelinePlayerCallback()
    {
        _timelineListModel->set(_current, _timelinePlayer->timelinePlayer());
    }

    qt::TimelinePlayer* App::_createTimelinePlayer(
        const std::string& fileName,
        const std::string& audioFileName)
    {
        qt::TimelinePlayer* out = nullptr;
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
            auto timeline = audioFileName.empty() ?
                timeline::Timeline::create(fileName, _context, options) :
                timeline::Timeline::create(fileName, audioFileName, _context, options);

            timeline::PlayerOptions playerOptions;
            playerOptions.timerMode = _settingsObject->timerMode();
            playerOptions.audioBufferFrameCount = _settingsObject->audioBufferFrameCount();
            auto timelinePlayer = timeline::TimelinePlayer::create(timeline, _context, playerOptions);

            out = new qt::TimelinePlayer(timelinePlayer, _context, this);

            connect(
                out,
                SIGNAL(speedChanged(double)),
                SLOT(_timelinePlayerCallback()));
            connect(
                out,
                SIGNAL(playbackChanged(tlr::timeline::Playback)),
                SLOT(_timelinePlayerCallback()));
            connect(
                out,
                SIGNAL(loopChanged(tlr::timeline::Loop)),
                SLOT(_timelinePlayerCallback()));
            connect(
                out,
                SIGNAL(currentTimeChanged(const otime::RationalTime&)),
                SLOT(_timelinePlayerCallback()));
            connect(
                out,
                SIGNAL(inOutRangeChanged(const otime::TimeRange&)),
                SLOT(_timelinePlayerCallback()));
            connect(
                out,
                SIGNAL(videoLayerChanged(int)),
                SLOT(_timelinePlayerCallback()));
            connect(
                out,
                SIGNAL(volumeChanged(float)),
                SLOT(_timelinePlayerCallback()));
            connect(
                out,
                SIGNAL(muteChanged(bool)),
                SLOT(_timelinePlayerCallback()));
            connect(
                out,
                SIGNAL(audioOffsetChanged(double)),
                SLOT(_timelinePlayerCallback()));
        }
        catch (const std::exception& e)
        {
            QMessageBox dialog;
            dialog.setText(e.what());
            dialog.exec();
        }
        return out;
    }

    qt::TimelinePlayer* App::_createTimelinePlayer(const TimelineListItem& item)
    {
        qt::TimelinePlayer* out = _createTimelinePlayer(item.path.get(), item.audioPath.get());
        if (out)
        {
            out->setAudioOffset(item.audioOffset);
            out->setMute(item.mute);
            out->setVolume(item.volume);
            out->setVideoLayer(item.videoLayer);
            out->setSpeed(item.speed);
            out->setLoop(item.loop);
            out->setInOutRange(item.inOutRange);
            out->seek(item.currentTime);
            out->setPlayback(item.playback);
        }
        return out;
    }
}
