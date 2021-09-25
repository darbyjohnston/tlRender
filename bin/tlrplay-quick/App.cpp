// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021 Darby Johnston
// All rights reserved.

#include "App.h"

#include <tlrQuick/GLFramebufferObject.h>

#include <tlrCore/Path.h>
#include <tlrCore/StringFormat.h>

#include <QQmlComponent>
#include <QQmlContext>

namespace tlr
{
    App::App(int& argc, char** argv) :
        QGuiApplication(argc, argv)
    {
        IApp::_init(
            argc,
            argv,
            "tlrplay-quick",
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
        QCoreApplication::setApplicationName("tlrplay-quick");
        quick::setContext(_context);

        // Create objects.
        _timeObject = new qt::TimeObject(this);

        // Open the input file.
        _timelinePlayer = new qt::TimelinePlayer(file::Path(_input), _context);

        // Load the QML.
        _qmlEngine = new QQmlApplicationEngine;
        _qmlEngine->rootContext()->setContextProperty("timelinePlayer", _timelinePlayer);
        QQmlComponent component(_qmlEngine, QUrl(QStringLiteral("qrc:/tlrplay-quick.qml")));
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
