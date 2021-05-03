// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021 Darby Johnston
// All rights reserved.

#pragma once

#include "GLWidget.h"
#include "MainWindow.h"

#include <tlrApp/IApp.h>

#include <tlrRender/Timeline.h>

#include <QApplication>
#include <QPointer>

namespace tlr
{
    //! Application.
    class App : public QApplication, public app::IApp
    {
        Q_OBJECT

    public:
        App(int& argc, char** argv);

    protected:
        void timerEvent(QTimerEvent*) override;

    private Q_SLOTS:
        void _fileOpenCallback();
        void _fileOpenCallback(const QString&);
        void _fileCloseCallback();

    private:
        void _fileOpen(const std::string&);

        std::string _input;
        std::shared_ptr<timeline::Timeline> _timeline;

        QPointer< GLWidget> _glWidget;
        QPointer<MainWindow> _mainWindow;

        std::shared_ptr<Observer::Value<std::shared_ptr<imaging::Image> > > _currentImageObserver;
    };
}
