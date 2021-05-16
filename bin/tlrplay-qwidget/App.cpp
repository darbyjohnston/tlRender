// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021 Darby Johnston
// All rights reserved.

#include "App.h"

#include <tlrCore/File.h>
#include <tlrCore/Math.h>
#include <tlrCore/String.h>
#include <tlrCore/StringFormat.h>
#include <tlrCore/Time.h>

#include <QFileDialog>
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

        connect(
            _mainWindow,
            SIGNAL(fileOpen()),
            SLOT(_fileOpenCallback()));
        connect(
            _mainWindow,
            SIGNAL(fileOpen(const QString&)),
            SLOT(_fileOpenCallback(const QString&)));
        connect(
            _mainWindow,
            SIGNAL(fileClose()),
            SLOT(_fileClose()));
        connect(
            _mainWindow,
            SIGNAL(exit()),
            SLOT(quit()));

        if (!_input.empty())
        {
            _fileOpen(_input.c_str());
        }

        _mainWindow->show();
    }

    void App::_fileOpenCallback()
    {
        std::string path;
        file::split(_input, &path);

        std::vector<std::string> extensions;
        for (const auto& i : timeline::getExtensions())
        {
            extensions.push_back("*" + i);
        }

        const auto fileName = QFileDialog::getOpenFileName(
            _mainWindow,
            tr("Open Timeline"),
            path.c_str(),
            tr("Timeline Files") + " (" + string::join(extensions, ", ").c_str() + ")");
        if (!fileName.isEmpty())
        {
            _fileOpen(fileName);
        }
    }

    void App::_fileOpenCallback(const QString& fileName)
    {
        _fileOpen(fileName);
    }

    void App::_fileOpen(const QString& fileName)
    {
        try
        {
            _mainWindow->setTimeline(nullptr);
            if (_timeline)
            {
                _timeline->setParent(nullptr);
                delete _timeline;
                _timeline = nullptr;
            }
            _input = fileName.toLatin1().data();
            _timeline = new qt::TimelineObject(fileName, this);
            _timeline->setFrameCacheReadAhead(_settingsObject->frameCacheReadAhead());
            _timeline->setFrameCacheReadBehind(_settingsObject->frameCacheReadBehind());
            _timeline->connect(
                _settingsObject,
                SIGNAL(frameCacheReadAheadChanged(int)),
                SLOT(setFrameCacheReadAhead(int)));
            _timeline->connect(
                _settingsObject,
                SIGNAL(frameCacheReadBehindChanged(int)),
                SLOT(setFrameCacheReadBehind(int)));
            _mainWindow->setTimeline(_timeline);
            //_timeline->setPlayback(timeline::Playback::Forward);

            _settingsObject->addRecentFile(fileName);
        }
        catch (const std::exception& e)
        {
            QMessageBox dialog;
            dialog.setText(e.what());
            dialog.exec();
        }
    }

    void App::_fileClose()
    {
        _mainWindow->setTimeline(nullptr);
        if (_timeline)
        {
            _timeline->setParent(nullptr);
            delete _timeline;
            _timeline = nullptr;
        }
    }
}
