// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021 Darby Johnston
// All rights reserved.

#include "App.h"

#include <tlrRender/Math.h>
#include <tlrRender/StringFormat.h>
#include <tlrRender/Time.h>

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
                    "The input timeline.")
            });
        if (getExit() != 0)
        {
            exit();
            return;
        }

        _timeline = timeline::Timeline::create(_input);
        _timeline->setPlayback(timeline::Playback::Forward);

        _glWidget = new GLWidget;

        _mainWindow = new QMainWindow;
        _mainWindow->setCentralWidget(_glWidget);
        const imaging::Info& info = _timeline->getImageInfo();
        _mainWindow->resize(info.size.w, info.size.h);
        _mainWindow->show();

        startTimer(0, Qt::PreciseTimer);
    }

    void App::timerEvent(QTimerEvent*)
    {
        if (_timeline)
        {
            _timeline->tick();
            auto image = _timeline->getCurrentImage();
            if (image != _currentImage)
            {
                _currentImage = image;
                _glWidget->setImage(image);
            }
        }
    }
}
