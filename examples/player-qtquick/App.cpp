// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2024 Darby Johnston
// All rights reserved.

#include "App.h"

#include <tlQtQuick/GLFramebufferObject.h>

#include <tlCore/AudioSystem.h>
#include <tlCore/Path.h>

#include <feather-tk/core/CmdLine.h>
#include <feather-tk/core/Format.h>

#include <QQmlComponent>
#include <QQmlContext>

namespace tl
{
    namespace examples
    {
        namespace player_qtquick
        {
            App::App(
                const std::shared_ptr<feather_tk::Context>& context,
                int& argc,
                char** argv) :
                QGuiApplication(argc, argv)
            {
                auto args = feather_tk::convert(argc, argv);
                IApp::_init(
                    context,
                    args,
                    "player-qtquick",
                    "Example Qt Quick player application.",
                    {
                        feather_tk::CmdLineValueArg<std::string>::create(
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
                setApplicationName("player-qtquick");

                // Create models and objects.
                _contextObject.reset(new qt::ContextObject(context, this));
                _timeUnitsModel = timeline::TimeUnitsModel::create(context);
                _timeObject.reset(new qt::TimeObject(_timeUnitsModel, this));

                // Open the input file.
                auto timeline = timeline::Timeline::create(context, _input);
                auto player = timeline::Player::create(context, timeline);
                _player.reset(new qt::PlayerObject(context, player));

                // Load the QML.
                _qmlEngine.reset(new QQmlApplicationEngine);
                _qmlEngine->rootContext()->setContextProperty("timelinePlayer", _player.get());
                QQmlComponent component(_qmlEngine.get(), QUrl(QStringLiteral("qrc:/player-qtquick.qml")));
                if (component.status() != QQmlComponent::Status::Ready)
                {
                    throw std::runtime_error(component.errorString().toUtf8().data());
                }
                _qmlObject.reset(component.create());

                // Start playback.
                _player->forward();
            }

            App::~App()
            {}
        }
    }
}
