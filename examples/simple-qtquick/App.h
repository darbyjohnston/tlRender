// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2023 Darby Johnston
// All rights reserved.

#pragma once

#include <tlApp/IApp.h>

#include <tlQt/ContextObject.h>
#include <tlQt/TimeObject.h>
#include <tlQt/TimelinePlayer.h>

#include <QGuiApplication>

#include <QQmlApplicationEngine>

namespace tl
{
    namespace examples
    {
        //! Example Qt Quick playback application.
        namespace simple_qtquick
        {
            //! Application.
            class App : public QGuiApplication, public app::IApp
            {
                Q_OBJECT

            public:
                App(
                    int& argc,
                    char** argv,
                    const std::shared_ptr<system::Context>&);
                ~App() override;

            private:
                std::string _input;

                qt::ContextObject* _contextObject = nullptr;
                std::shared_ptr<timeline::TimeUnitsModel> _timeUnitsModel;
                qt::TimeObject* _timeObject = nullptr;
                QScopedPointer<qt::TimelinePlayer> _timelinePlayer;

                QQmlApplicationEngine* _qmlEngine = nullptr;
                QObject* _qmlObject = nullptr;
            };
        }
    }
}
