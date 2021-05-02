// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021 Darby Johnston
// All rights reserved.

#include "GLWidget.h"

#include <tlrApp/IApp.h>

#include <tlrRender/Timeline.h>

#include <QApplication>
#include <QMainWindow>
#include <QPointer>

namespace tlr
{
    //! Application.
    class App : public QApplication, public app::IApp
    {
    public:
        App(int& argc, char** argv);

    protected:
        void timerEvent(QTimerEvent*) override;

    private:
        std::string _input;
        std::shared_ptr<timeline::Timeline> _timeline;

        std::shared_ptr<imaging::Image> _currentImage;
        QPointer< GLWidget> _glWidget;
        QPointer<QMainWindow> _mainWindow;
    };
}
