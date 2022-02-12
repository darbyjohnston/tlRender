// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2022 Darby Johnston
// All rights reserved.

#pragma once

#include <tlApp/IApp.h>

#include <tlQt/TimeObject.h>
#include <tlQt/TimelinePlayer.h>

#include <tlGL/Render.h>

#include <QGuiApplication>

#include <QQmlApplicationEngine>

namespace tl
{
    //! Application.
    class App : public QGuiApplication, public app::IApp
    {
        Q_OBJECT

    public:
        App(int& argc, char** argv);
        ~App() override;

    private:
        std::string _input;

        qt::TimeObject* _timeObject = nullptr;
        qt::TimelinePlayer* _timelinePlayer = nullptr;

        QQmlApplicationEngine* _qmlEngine = nullptr;
        QObject* _qmlObject = nullptr;
    };
}
