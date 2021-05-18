// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021 Darby Johnston
// All rights reserved.

#include "App.h"

#include <tlrCore/Math.h>
#include <tlrCore/StringFormat.h>
#include <tlrCore/Time.h>

#include <QMessageBox>

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
            });
        const int exitCode = getExit();
        if (exitCode != 0)
        {
            exit(exitCode);
            return;
        }

        Q_INIT_RESOURCE(tlrQt);

        qRegisterMetaType<qt::TimeObject::Units>("tlr::qt::TimeObject::Units");
        qRegisterMetaTypeStreamOperators<qt::TimeObject::Units>("tlr::qt::TimeObject::Units");

        QCoreApplication::setOrganizationName("tlRender");
        QCoreApplication::setApplicationName("tlrplay-qwidget");

        setStyle("Fusion");

        _timeObject = new qt::TimeObject(this);
        _settingsObject = new SettingsObject(_timeObject, this);

        _mainWindow = new MainWindow(_settingsObject, _timeObject);

        if (!_input.empty())
        {
            open(_input.c_str());
        }

        _mainWindow->show();
    }

    void App::open(const QString& fileName)
    {
        try
        {
            auto timeline = new qt::TimelineObject(fileName, this);
            timeline->setFrameCacheReadAhead(_settingsObject->frameCacheReadAhead());
            timeline->setFrameCacheReadBehind(_settingsObject->frameCacheReadBehind());
            timeline->connect(
                _settingsObject,
                SIGNAL(frameCacheReadAheadChanged(int)),
                SLOT(setFrameCacheReadAhead(int)));
            timeline->connect(
                _settingsObject,
                SIGNAL(frameCacheReadBehindChanged(int)),
                SLOT(setFrameCacheReadBehind(int)));
            _timelines.append(timeline);

            Q_EMIT opened(timeline);

            _settingsObject->addRecentFile(fileName);
        }
        catch (const std::exception& e)
        {
            QMessageBox dialog;
            dialog.setText(e.what());
            dialog.exec();
        }
    }

    void App::close(qt::TimelineObject* timeline)
    {
        const int i = _timelines.indexOf(timeline);
        if (i != -1)
        {
            _timelines.removeAt(i);
            Q_EMIT closed(timeline);
            timeline->setParent(nullptr);
            delete timeline;
        }
    }

    void App::closeAll()
    {
        while (!_timelines.empty())
        {
            close(_timelines.back());
        }
    }
}
