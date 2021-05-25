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
            "tlrfilmstrip-qwidget",
            "View a timeline as a series of thumbnail images.",
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

        qtInitResources();

        qRegisterMetaType<qt::TimeObject::Units>("tlr::qt::TimeObject::Units");
        qRegisterMetaTypeStreamOperators<qt::TimeObject::Units>("tlr::qt::TimeObject::Units");

        QCoreApplication::setOrganizationName("tlRender");
        QCoreApplication::setApplicationName("tlrfilmstrip-qwidget");

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
            auto timeline = timeline::Timeline::create(fileName.toLatin1().data());
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

    void App::close(const std::shared_ptr<timeline::Timeline>& timeline)
    {
        const int i = _timelines.indexOf(timeline);
        if (i != -1)
        {
            _timelines.removeAt(i);
            Q_EMIT closed(timeline);
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
