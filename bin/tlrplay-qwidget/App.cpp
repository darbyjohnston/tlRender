// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021 Darby Johnston
// All rights reserved.

#include "App.h"

#include <tlrCore/Math.h>
#include <tlrCore/StringFormat.h>
#include <tlrCore/Time.h>

#include <QMessageBox>

void qtInitResources()
{
    Q_INIT_RESOURCE(tlrQt);
}

namespace tlr
{
    App::App(int& argc, char** argv) :
        QApplication(argc, argv)
    {
        IApp::_init(
            argc,
            argv,
            "tlrplay-qwidget",
            "Play an editorial timeline.",
            {
                app::CmdLineValueArg<std::string>::create(
                    _input,
                    "Input",
                    "The input timeline.",
                    true)
            },
            {
                app::CmdLineValueOption<std::string>::create(
                    _options.colorConfig.config,
                    { "-colorConfig", "-cc" },
                    "Color configuration."),
                app::CmdLineValueOption<std::string>::create(
                    _options.colorConfig.input,
                    { "-colorInput", "-ci" },
                    "Input color space."),
                app::CmdLineValueOption<std::string>::create(
                    _options.colorConfig.display,
                    { "-colorDisplay", "-cd" },
                    "Display color space."),
                app::CmdLineValueOption<std::string>::create(
                    _options.colorConfig.view,
                    { "-colorView", "-cv" },
                    "View color space.")
            });
        const int exitCode = getExit();
        if (exitCode != 0)
        {
            exit(exitCode);
            return;
        }

        // Initialize Qt.
        qtInitResources();
        qRegisterMetaType<qt::TimeObject::Units>("tlr::qt::TimeObject::Units");
        qRegisterMetaTypeStreamOperators<qt::TimeObject::Units>("tlr::qt::TimeObject::Units");
        QCoreApplication::setOrganizationName("tlRender");
        QCoreApplication::setApplicationName("tlrplay-qwidget");
        setStyle("Fusion");

        // Create objects.
        _timeObject = new qt::TimeObject(this);
        _settingsObject = new SettingsObject(_timeObject, this);

        // Create the main window.
        _mainWindow = new MainWindow(_settingsObject, _timeObject);
        _mainWindow->setColorConfig(_options.colorConfig);

        // Open the input file.
        if (!_input.empty())
        {
            open(_input.c_str());
        }

        _mainWindow->show();
    }

    App::~App()
    {
        _settingsObject->setParent(nullptr);
        delete _settingsObject;
    }

    void App::open(const QString& fileName)
    {
        try
        {
            auto timelinePlayer = new qt::TimelinePlayer(fileName, this);
            timelinePlayer->setFrameCacheReadAhead(_settingsObject->frameCacheReadAhead());
            timelinePlayer->setFrameCacheReadBehind(_settingsObject->frameCacheReadBehind());
            timelinePlayer->connect(
                _settingsObject,
                SIGNAL(frameCacheReadAheadChanged(int)),
                SLOT(setFrameCacheReadAhead(int)));
            timelinePlayer->connect(
                _settingsObject,
                SIGNAL(frameCacheReadBehindChanged(int)),
                SLOT(setFrameCacheReadBehind(int)));
            _timelinePlayers.append(timelinePlayer);

            Q_EMIT opened(timelinePlayer);

            _settingsObject->addRecentFile(fileName);
        }
        catch (const std::exception& e)
        {
            QMessageBox dialog;
            dialog.setText(e.what());
            dialog.exec();
        }
    }

    void App::close(qt::TimelinePlayer* timelinePlayer)
    {
        const int i = _timelinePlayers.indexOf(timelinePlayer);
        if (i != -1)
        {
            _timelinePlayers.removeAt(i);
            Q_EMIT closed(timelinePlayer);
            timelinePlayer->setParent(nullptr);
            delete timelinePlayer;
        }
    }

    void App::closeAll()
    {
        while (!_timelinePlayers.empty())
        {
            close(_timelinePlayers.back());
        }
    }
}
