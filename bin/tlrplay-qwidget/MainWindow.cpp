// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021 Darby Johnston
// All rights reserved.

#include "MainWindow.h"

#include <QDragEnterEvent>
#include <QDragMoveEvent>
#include <QFontDatabase>
#include <QHBoxLayout>
#include <QMenuBar>
#include <QMimeData>
#include <QStyle>
#include <QToolBar>

namespace tlr
{
    MainWindow::MainWindow(QWidget* parent) :
        QMainWindow(parent)
    {
        setAcceptDrops(true);

        _actions["File/Open"] = new QAction;
        _actions["File/Open"]->setText(tr("Open"));
        _actions["File/Open"]->setShortcut(QKeySequence::Open);
        _actions["File/Close"] = new QAction;
        _actions["File/Close"]->setText(tr("Close"));
        _actions["File/Close"]->setShortcut(QKeySequence::Close);
        _actions["File/Exit"] = new QAction;
        _actions["File/Exit"]->setText(tr("Exit"));
        _actions["File/Exit"]->setShortcut(QKeySequence::Quit);

        _actions["Playback/Stop"] = new QAction;
        _actions["Playback/Stop"]->setCheckable(true);
        _actions["Playback/Stop"]->setText(tr("Stop Playback"));
        _actions["Playback/Stop"]->setIcon(style()->standardIcon(QStyle::SP_MediaStop));
        _actions["Playback/Stop"]->setToolTip(tr("Stop playback"));
        _actions["Playback/Forward"] = new QAction;
        _actions["Playback/Forward"]->setCheckable(true);
        _actions["Playback/Forward"]->setText(tr("Forward Playback"));
        _actions["Playback/Forward"]->setIcon(style()->standardIcon(QStyle::SP_MediaPlay));
        _actions["Playback/Forward"]->setToolTip(tr("Forward playback"));
        _actions["Playback/Toggle"] = new QAction;
        _actions["Playback/Toggle"]->setText(tr("Toggle Playback"));
        _actions["Playback/Toggle"]->setShortcut(QKeySequence(Qt::Key_Space));
        _actions["Playback/Toggle"]->setToolTip(tr("Toggle playback"));

        _actions["Playback/StartFrame"] = new QAction;
        _actions["Playback/StartFrame"]->setText(tr("Start Frame"));
        _actions["Playback/StartFrame"]->setShortcut(QKeySequence(Qt::Key_Home));
        _actions["Playback/EndFrame"] = new QAction;
        _actions["Playback/EndFrame"]->setText(tr("End Frame"));
        _actions["Playback/EndFrame"]->setShortcut(QKeySequence(Qt::Key_End));
        _actions["Playback/PrevFrame"] = new QAction;
        _actions["Playback/PrevFrame"]->setText(tr("Previous Frame"));
        _actions["Playback/PrevFrame"]->setShortcut(QKeySequence(Qt::Key_Left));
        _actions["Playback/NextFrame"] = new QAction;
        _actions["Playback/NextFrame"]->setText(tr("Next Frame"));
        _actions["Playback/NextFrame"]->setShortcut(QKeySequence(Qt::Key_Right));

        auto fileMenu = new QMenu;
        fileMenu->setTitle(tr("&File"));
        fileMenu->addAction(_actions["File/Open"]);
        fileMenu->addAction(_actions["File/Close"]);
        fileMenu->addSeparator();
        fileMenu->addAction(_actions["File/Exit"]);

        auto playbackMenu = new QMenu;
        playbackMenu->setTitle(tr("&Playback"));
        playbackMenu->addAction(_actions["Playback/Toggle"]);
        playbackMenu->addSeparator();
        playbackMenu->addAction(_actions["Playback/StartFrame"]);
        playbackMenu->addAction(_actions["Playback/EndFrame"]);
        playbackMenu->addAction(_actions["Playback/PrevFrame"]);
        playbackMenu->addAction(_actions["Playback/NextFrame"]);

        auto menuBar = new QMenuBar;
        menuBar->addMenu(fileMenu);
        menuBar->addMenu(playbackMenu);
        setMenuBar(menuBar);

        _timelineWidget = new qt::TimelineWidget;
        setCentralWidget(_timelineWidget);

        _playbackUpdate();
        _timelineUpdate();

        connect(
            _actions["File/Open"],
            SIGNAL(triggered()),
            SIGNAL(fileOpen()));
        connect(
            _actions["File/Close"],
            SIGNAL(triggered()),
            SIGNAL(fileClose()));
        connect(
            _actions["File/Exit"],
            SIGNAL(triggered()),
            SIGNAL(exit()));

        connect(
            _actions["Playback/Stop"],
            SIGNAL(triggered()),
            SLOT(_stopCallback()));
        connect(
            _actions["Playback/Forward"],
            SIGNAL(triggered()),
            SLOT(_forwardCallback()));
        connect(
            _actions["Playback/Toggle"],
            SIGNAL(triggered()),
            SLOT(_togglePlaybackCallback()));
        connect(
            _actions["Playback/StartFrame"],
            SIGNAL(triggered()),
            SLOT(_startFrameCallback()));
        connect(
            _actions["Playback/EndFrame"],
            SIGNAL(triggered()),
            SLOT(_endFrameCallback()));
        connect(
            _actions["Playback/PrevFrame"],
            SIGNAL(triggered()),
            SLOT(_prevFrameCallback()));
        connect(
            _actions["Playback/NextFrame"],
            SIGNAL(triggered()),
            SLOT(_nextFrameCallback()));
    }

    void MainWindow::setTimeline(qt::TimelineObject* timeline)
    {
        if (timeline == _timeline)
            return;
        _timeline = timeline;
        _timelineWidget->setTimeline(timeline);
        _timelineUpdate();
    }

    void MainWindow::dragEnterEvent(QDragEnterEvent* event)
    {
        const QMimeData* mimeData = event->mimeData();
        if (mimeData->hasUrls())
        {
            const auto urlList = mimeData->urls();
            if (1 == urlList.size())
            {
                event->acceptProposedAction();
            }
        }
    }

    void MainWindow::dragMoveEvent(QDragMoveEvent* event)
    {
        const QMimeData* mimeData = event->mimeData();
        if (mimeData->hasUrls())
        {
            const auto urlList = mimeData->urls();
            if (1 == urlList.size())
            {
                event->acceptProposedAction();
            }
        }
    }

    void MainWindow::dragLeaveEvent(QDragLeaveEvent* event)
    {
        event->accept();
    }

    void MainWindow::dropEvent(QDropEvent* event)
    {
        const QMimeData* mimeData = event->mimeData();
        if (mimeData->hasUrls())
        {
            const auto urlList = mimeData->urls();
            if (1 == urlList.size())
            {
                fileOpen(urlList[0].toLocalFile().toLatin1().data());
            }
        }
    }

    void MainWindow::_playbackCallback(tlr::timeline::Playback value)
    {
        if (_timeline)
        {
            _timeline->setPlayback(value);
        }
    }

    void MainWindow::_loopCallback(tlr::timeline::Loop value)
    {
        if (_timeline)
        {
            _timeline->setLoop(value);
        }
    }

    void MainWindow::_stopCallback()
    {
        if (_timeline)
        {
            _timeline->stop();
        }
    }

    void MainWindow::_forwardCallback()
    {
        if (_timeline)
        {
            _timeline->forward();
        }
    }

    void MainWindow::_togglePlaybackCallback()
    {
        if (_timeline)
        {
            _timeline->togglePlayback();
        }
    }

    void MainWindow::_startFrameCallback()
    {
        if (_timeline)
        {
            _timeline->startFrame();
        }
    }

    void MainWindow::_endFrameCallback()
    {
        if (_timeline)
        {
            _timeline->endFrame();
        }
    }

    void MainWindow::_prevFrameCallback()
    {
        if (_timeline)
        {
            _timeline->prevFrame();
        }
    }

    void MainWindow::_nextFrameCallback()
    {
        if (_timeline)
        {
            _timeline->nextFrame();
        }
    }

    void MainWindow::_playbackUpdate()
    {
        timeline::Playback playback = timeline::Playback::Stop;
        if (_timeline)
        {
            playback = _timeline->getPlayback();
        }
        _actions["Playback/Stop"]->setChecked(timeline::Playback::Stop == playback);
        _actions["Playback/Forward"]->setChecked(timeline::Playback::Forward == playback);
    }

    void MainWindow::_timelineUpdate()
    {
        if (_timeline)
        {
            _actions["File/Close"]->setEnabled(true);

            _actions["Playback/Stop"]->setEnabled(true);
            _actions["Playback/Forward"]->setEnabled(true);
            _actions["Playback/StartFrame"]->setEnabled(true);
            _actions["Playback/EndFrame"]->setEnabled(true);
            _actions["Playback/PrevFrame"]->setEnabled(true);
            _actions["Playback/NextFrame"]->setEnabled(true);
        }
        else
        {
            _actions["File/Close"]->setEnabled(false);

            _actions["Playback/Stop"]->setChecked(true);
            _actions["Playback/Stop"]->setEnabled(false);
            _actions["Playback/Forward"]->setEnabled(false);
            _actions["Playback/StartFrame"]->setEnabled(false);
            _actions["Playback/EndFrame"]->setEnabled(false);
            _actions["Playback/PrevFrame"]->setEnabled(false);
            _actions["Playback/NextFrame"]->setEnabled(false);
        }
    }
}
