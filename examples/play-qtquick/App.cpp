// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2023 Darby Johnston
// All rights reserved.

#include "App.h"

#include <tlQtQuick/GLFramebufferObject.h>

#include <tlCore/AudioSystem.h>
#include <tlCore/Path.h>
#include <tlCore/StringFormat.h>

#include <QQmlComponent>
#include <QQmlContext>

namespace tl
{
    namespace examples
    {
        namespace play_qtquick
        {
            App::App(
                int& argc,
                char** argv,
                const std::shared_ptr<system::Context>& context) :
                QGuiApplication(argc, argv)
            {
                IApp::_init(
                    argc,
                    argv,
                    context,
                    "play-qtquick",
                    "Example Qt Quick playback application.",
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
                setOrganizationName("tlRender");
                setApplicationName("play-qtquick");

                // Create objects.
                _contextObject = new qt::ContextObject(context, this);
                _timeObject = new qt::TimeObject(this);

                // Open the input file.
                auto timeline = timeline::Timeline::create(_input, context);
                _timelinePlayer = new qt::TimelinePlayer(timeline::TimelinePlayer::create(timeline, context), context);

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
