// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021 Darby Johnston
// All rights reserved.

#pragma once

#include <tlrApp/IApp.h>

#include <tlrQt/TimeObject.h>
#include <tlrQt/TimelinePlayer.h>

#include <tlrGL/Render.h>

#include <QGuiApplication>

#include <QQmlApplicationEngine>

namespace tlr
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
