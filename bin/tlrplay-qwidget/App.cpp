// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021 Darby Johnston
// All rights reserved.

#include "App.h"

#include <tlrRender/File.h>
#include <tlrRender/Math.h>
#include <tlrRender/String.h>
#include <tlrRender/StringFormat.h>
#include <tlrRender/Time.h>

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

        _glWidget = new GLWidget;

        _mainWindow = new MainWindow;
        _mainWindow->setCentralWidget(_glWidget);

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
            SLOT(_fileCloseCallback()));
        connect(
            _mainWindow,
            SIGNAL(exit()),
            SLOT(quit()));

        if (!_input.empty())
        {
            _fileOpen(_input);
        }

        _mainWindow->resize(640, 360);
        _mainWindow->show();

        startTimer(0, Qt::PreciseTimer);
    }

    void App::_fileOpen(const std::string& fileName)
    {
        try
        {
            _timeline = timeline::Timeline::create(fileName);

            _input = fileName;

            _mainWindow->setTimeline(_timeline);

            _currentImageObserver = Observer::Value<std::shared_ptr<imaging::Image> >::create(
                _timeline->observeCurrentImage(),
                [this](const std::shared_ptr<imaging::Image>& value)
                {
                    _glWidget->setImage(value);
                });

            _timeline->setPlayback(timeline::Playback::Forward);
        }
        catch (const std::exception& e)
        {
            QMessageBox dialog;
            dialog.setText(e.what());
            dialog.exec();
        }
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
            _fileOpen(fileName.toLatin1().data());
        }
    }

    void App::_fileOpenCallback(const QString& fileName)
    {
        _fileOpen(fileName.toLatin1().data());
    }

    void App::_fileCloseCallback()
    {
        _timeline.reset();
        _mainWindow->setTimeline(nullptr);
        _glWidget->setImage(nullptr);
        _currentImageObserver.reset();
    }

    void App::timerEvent(QTimerEvent*)
    {
        if (_timeline)
        {
            _timeline->tick();
        }
    }
}
