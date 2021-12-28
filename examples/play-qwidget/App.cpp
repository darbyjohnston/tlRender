// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021 Darby Johnston
// All rights reserved.

#include "App.h"

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

    void App::open(const QString& fileName)
    {
        try
        {
            timeline::PlayerOptions playerOptions;
            playerOptions.timerMode = _settingsObject->timerMode();
            playerOptions.audioBufferFrameCount = _settingsObject->audioBufferFrameCount();
            timeline::Options options;
            options.fileSequenceAudio = _settingsObject->fileSequenceAudio();
            options.fileSequenceAudioFileName = _settingsObject->fileSequenceAudioFileName().toUtf8().data();
            options.fileSequenceAudioDirectory = _settingsObject->fileSequenceAudioDirectory().toUtf8().data();
            options.videoRequestCount = _settingsObject->videoRequestCount();
            options.audioRequestCount = _settingsObject->audioRequestCount();
            options.avioOptions["SequenceIO/ThreadCount"] = string::Format("{0}").arg(_settingsObject->sequenceThreadCount());
            options.avioOptions["ffmpeg/ThreadCount"] = string::Format("{0}").arg(_settingsObject->ffmpegThreadCount());
            options.pathOptions.maxNumberDigits = std::min(_settingsObject->maxFileSequenceDigits(), 255);
            auto timeline = timeline::Timeline::create(
                fileName.toUtf8().data(),
                _context,
                options);
            auto timelinePlayer = new qt::TimelinePlayer(
                timeline::TimelinePlayer::create(timeline, _context, playerOptions),
                _context,
                this);
            _settingsUpdate(timelinePlayer);
            _timelinePlayers.append(timelinePlayer);

            Q_EMIT opened(timelinePlayer);

            _settingsObject->addRecentFile(fileName);
        }
        catch (const std::exception& e)
        {
            QMessageBox dialog;
            dialog.setText(e.what());
            dialog.exec();
        }
    }

    void App::openPlusAudio(const QString& fileName, const QString& audioFileName)
    {
        try
        {
            timeline::PlayerOptions playerOptions;
            playerOptions.timerMode = _settingsObject->timerMode();
            playerOptions.audioBufferFrameCount = _settingsObject->audioBufferFrameCount();
            timeline::Options options;
            options.videoRequestCount = _settingsObject->videoRequestCount();
            options.audioRequestCount = _settingsObject->audioRequestCount();
            options.avioOptions["SequenceIO/ThreadCount"] = string::Format("{0}").arg(_settingsObject->sequenceThreadCount());
            options.avioOptions["ffmpeg/ThreadCount"] = string::Format("{0}").arg(_settingsObject->ffmpegThreadCount());
            auto timeline = timeline::Timeline::create(
                fileName.toUtf8().data(),
                audioFileName.toUtf8().data(),
                _context,
                options);
            auto timelinePlayer = new qt::TimelinePlayer(
                timeline::TimelinePlayer::create(timeline, _context, playerOptions),
                _context,
                this);
            _settingsUpdate(timelinePlayer);
            _timelinePlayers.append(timelinePlayer);

            Q_EMIT opened(timelinePlayer);

            _settingsObject->addRecentFile(fileName);
        }
        catch (const std::exception& e)
        {
            QMessageBox dialog;
            dialog.setText(e.what());
            dialog.exec();
        }
    }

    void App::close(qt::TimelinePlayer* timelinePlayer)
    {
        const int i = _timelinePlayers.indexOf(timelinePlayer);
        if (i != -1)
        {
            _timelinePlayers.removeAt(i);
            Q_EMIT closed(timelinePlayer);
            timelinePlayer->setParent(nullptr);
            delete timelinePlayer;
        }
    }

    void App::closeAll()
    {
        while (!_timelinePlayers.empty())
        {
            close(_timelinePlayers.back());
        }
    }

    void App::_settingsCallback()
    {
        for (auto i : _timelinePlayers)
        {
            _settingsUpdate(i);
        }
    }

    void App::_settingsUpdate(qt::TimelinePlayer* value)
    {
        value->setCacheReadAhead(otime::RationalTime(_settingsObject->cacheReadAhead(), 1.0));
        value->setCacheReadBehind(otime::RationalTime(_settingsObject->cacheReadBehind(), 1.0));
    }
}
