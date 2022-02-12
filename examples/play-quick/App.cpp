// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2022 Darby Johnston
// All rights reserved.

#include "App.h"

#include <tlQuick/GLFramebufferObject.h>

#include <tlCore/AudioSystem.h>
#include <tlCore/Path.h>
#include <tlCore/StringFormat.h>

#include <QQmlComponent>
#include <QQmlContext>

namespace tl
{
    App::App(int& argc, char** argv) :
        QGuiApplication(argc, argv)
    {
        IApp::_init(
            argc,
            argv,
            "play-quick",
            "Play an editorial timeline.",
            {
                app::CmdLineValueArg<std::string>::create(
                    _input,
                    "input",
                    "The input timeline.")
            });
        const int exitCode = getExit();
        if (exitCode != 0)
        {
            exit(exitCode);
            return;
        }

        // Initialize Qt.
        QCoreApplication::setOrganizationName("tlRender");
        QCoreApplication::setApplicationName("play-quick");
        quick::setContext(_context);

        // Create objects.
        _timeObject = new qt::TimeObject(this);

        // Open the input file.
        timeline::Options options;
        auto audioSystem = _context->getSystem<audio::System>();
        const audio::Info audioInfo = audioSystem->getDefaultOutputInfo();
        options.avioOptions["ffmpeg/AudioChannelCount"] = string::Format("{0}").arg(audioInfo.channelCount);
        options.avioOptions["ffmpeg/AudioDataType"] = string::Format("{0}").arg(audioInfo.dataType);
        options.avioOptions["ffmpeg/AudioSampleRate"] = string::Format("{0}").arg(audioInfo.sampleRate);
        auto timeline = timeline::Timeline::create(_input, _context, options);
        _timelinePlayer = new qt::TimelinePlayer(timeline::TimelinePlayer::create(timeline, _context), _context);

        // Load the QML.
        _qmlEngine = new QQmlApplicationEngine;
        _qmlEngine->rootContext()->setContextProperty("timelinePlayer", _timelinePlayer);
        QQmlComponent component(_qmlEngine, QUrl(QStringLiteral("qrc:/play-quick.qml")));
        if (component.status() != QQmlComponent::Status::Ready)
        {
            throw std::runtime_error(component.errorString().toUtf8().data());
        }
        _qmlObject = component.create();

        // Start playback.
        _timelinePlayer->setPlayback(timeline::Playback::Forward);
    }

    App::~App()
    {}
}
