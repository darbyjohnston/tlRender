// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2024 Darby Johnston
// All rights reserved.

#pragma once

#include <tlQt/ContextObject.h>
#include <tlQt/TimeObject.h>
#include <tlQt/TimelinePlayer.h>

#include <dtk/core/IApp.h>

#include <QGuiApplication>

#include <QQmlApplicationEngine>

namespace tl
{
    namespace examples
    {
        //! Example Qt Quick player application.
        namespace player_qtquick
        {
            //! Application.
            class App : public QGuiApplication, public dtk::IApp
            {
                Q_OBJECT

            public:
                App(
                    const std::shared_ptr<dtk::Context>&,
                    int& argc,
                    char** argv);
                
                virtual ~App();

            private:
                std::string _input;

                QScopedPointer<qt::ContextObject> _contextObject;
                std::shared_ptr<timeline::TimeUnitsModel> _timeUnitsModel;
                QScopedPointer<qt::TimeObject> _timeObject;
                QScopedPointer<qt::TimelinePlayer> _timelinePlayer;

                QScopedPointer<QQmlApplicationEngine> _qmlEngine;
                QScopedPointer<QObject> _qmlObject;
            };
        }
    }
}
