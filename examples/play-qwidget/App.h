// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2022 Darby Johnston
// All rights reserved.

#pragma once

#include "MainWindow.h"
#include "SettingsObject.h"
#include "TimelineListModel.h"

#include <tlrApp/IApp.h>

#include <tlrQt/TimeObject.h>
#include <tlrQt/TimelinePlayer.h>

#include <tlrCore/OCIO.h>

#include <QApplication>

namespace tlr
{
    //! Application options.
    struct Options
    {
        imaging::ColorConfig colorConfig;
    };

    //! Application.
    class App : public QApplication, public app::IApp
    {
        Q_OBJECT

    public:
        App(int& argc, char** argv);
        ~App() override;

        //! Get the timeline model.
        TimelineListModel* timelineListModel() const;

        //! Get the current timeline player.
        int current() const;

    public Q_SLOTS:
        //! Open a timeline.
        void open(const QString&);

        //! Open a timeline with audio.
        void openWithAudio(const QString&, const QString&);

        //! Close the current timeline.
        void close();

        //! Close all timelines.
        void closeAll();

        //! Set the current timeline player.
        void setCurrent(int);

    private Q_SLOTS:
        void _timelinePlayerCallback();

    private:
        qt::TimelinePlayer* _createTimelinePlayer(
            const std::string& fileName,
            const std::string& audioFileName = std::string());
        qt::TimelinePlayer* _createTimelinePlayer(const TimelineListItem&);

        std::string _input;
        Options _options;

        qt::TimeObject* _timeObject = nullptr;
        SettingsObject* _settingsObject = nullptr;

        qt::TimelinePlayer* _timelinePlayer = nullptr;
        TimelineListModel* _timelineListModel = nullptr;
        int _current = -1;

        MainWindow* _mainWindow = nullptr;
    };
}
