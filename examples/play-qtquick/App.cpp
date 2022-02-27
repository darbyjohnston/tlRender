// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2022 Darby Johnston
// All rights reserved.

#include "App.h"

#include <tlQtQuick/GLFramebufferObject.h>

#include <tlCore/AudioSystem.h>
#include <tlCore/Path.h>
#include <tlCore/StringFormat.h>

#include <QQmlComponent>
#include <QQmlContext>

using namespace tl::core;

namespace tl
{
    namespace examples
    {
        namespace play_qtquick
        {
            App::App(int& argc, char** argv) :
                QGuiApplication(argc, argv)
            {
                IApp::_init(
                    argc,
                    argv,
                    "play-qtquick",
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
                QCoreApplication::setApplicationName("play-qtquick");
                qt::quick::setContext(_context);

                // Create objects.
                _timeObject = new qt::TimeObject(this);

                // Open the input file.
                timeline::Options options;
                auto audioSystem = _context->getSystem<audio::System>();
                const audio::Info audioInfo = audioSystem->getDefaultOutputInfo();
                options.ioOptions["ffmpeg/AudioChannelCount"] = string::Format("{0}").arg(audioInfo.channelCount);
                options.ioOptions["ffmpeg/AudioDataType"] = string::Format("{0}").arg(audioInfo.dataType);
                options.ioOptions["ffmpeg/AudioSampleRate"] = string::Format("{0}").arg(audioInfo.sampleRate);
                auto timeline = timeline::Timeline::create(_input, _context, options);
                _timelinePlayer = new qt::TimelinePlayer(timeline::TimelinePlayer::create(timeline, _context), _context);

                // Load the QML.
                _qmlEngine = new QQmlApplicationEngine;
                _qmlEngine->rootContext()->setContextProperty("timelinePlayer", _timelinePlayer);
                QQmlComponent component(_qmlEngine, QUrl(QStringLiteral("qrc:/play-qtquick.qml")));
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
    }
}
