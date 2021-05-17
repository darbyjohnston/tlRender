// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021 Darby Johnston
// All rights reserved.

#include "MainWindow.h"

#include "SettingsWidget.h"

#include <QDockWidget>
#include <QDragEnterEvent>
#include <QDragMoveEvent>
#include <QFontDatabase>
#include <QHBoxLayout>
#include <QMenuBar>
#include <QMimeData>
#include <QSettings>
#include <QStyle>
#include <QToolBar>

namespace tlr
{
    MainWindow::MainWindow(
        SettingsObject* settingsObject,
        qt::TimeObject* timeObject,
        QWidget* parent) :
        QMainWindow(parent),
        _settingsObject(settingsObject),
        _timeObject(timeObject)
    {
        setFocusPolicy(Qt::ClickFocus);
        setAcceptDrops(true);

        _actions["File/Open"] = new QAction(this);
        _actions["File/Open"]->setText(tr("Open"));
        _actions["File/Open"]->setShortcut(QKeySequence::Open);
        _actions["File/Close"] = new QAction(this);
        _actions["File/Close"]->setText(tr("Close"));
        _actions["File/Close"]->setShortcut(QKeySequence::Close);
        _recentFilesActionGroup = new QActionGroup(this);
        _actions["File/Settings"] = new QAction(this);
        _actions["File/Settings"]->setText(tr("Settings"));
        _actions["File/Settings"]->setCheckable(true);
        _actions["File/Exit"] = new QAction(this);
        _actions["File/Exit"]->setText(tr("Exit"));
        _actions["File/Exit"]->setShortcut(QKeySequence::Quit);

        _actions["Playback/Stop"] = new QAction(this);
        _actions["Playback/Stop"]->setCheckable(true);
        _actions["Playback/Stop"]->setText(tr("Stop Playback"));
        _actions["Playback/Stop"]->setIcon(QIcon(":/Icons/PlaybackStop.svg"));
        _actions["Playback/Stop"]->setShortcut(QKeySequence(Qt::Key_K));
        _actions["Playback/Stop"]->setToolTip(tr("Stop playback"));
        _actions["Playback/Forward"] = new QAction(this);
        _actions["Playback/Forward"]->setCheckable(true);
        _actions["Playback/Forward"]->setText(tr("Forward Playback"));
        _actions["Playback/Forward"]->setIcon(QIcon(":/Icons/PlaybackForward.svg"));
        _actions["Playback/Forward"]->setShortcut(QKeySequence(Qt::Key_L));
        _actions["Playback/Forward"]->setToolTip(tr("Forward playback"));
        _actions["Playback/Reverse"] = new QAction(this);
        _actions["Playback/Reverse"]->setCheckable(true);
        _actions["Playback/Reverse"]->setText(tr("Reverse Playback"));
        _actions["Playback/Reverse"]->setIcon(QIcon(":/Icons/PlaybackReverse.svg"));
        _actions["Playback/Reverse"]->setShortcut(QKeySequence(Qt::Key_J));
        _actions["Playback/Reverse"]->setToolTip(tr("Reverse playback"));
        _playbackActionGroup = new QActionGroup(this);
        _playbackActionGroup->setExclusive(true);
        _playbackActionGroup->addAction(_actions["Playback/Stop"]);
        _playbackActionGroup->addAction(_actions["Playback/Forward"]);
        _playbackActionGroup->addAction(_actions["Playback/Reverse"]);
        _actionToPlayback[_actions["Playback/Stop"]] = timeline::Playback::Stop;
        _actionToPlayback[_actions["Playback/Forward"]] = timeline::Playback::Forward;
        _actionToPlayback[_actions["Playback/Reverse"]] = timeline::Playback::Reverse;
        _playbackToActions[timeline::Playback::Stop] = _actions["Playback/Stop"];
        _playbackToActions[timeline::Playback::Forward] = _actions["Playback/Forward"];
        _playbackToActions[timeline::Playback::Reverse] = _actions["Playback/Reverse"];
        _actions["Playback/Toggle"] = new QAction(this);
        _actions["Playback/Toggle"]->setText(tr("Toggle Playback"));
        _actions["Playback/Toggle"]->setShortcut(QKeySequence(Qt::Key_Space));
        _actions["Playback/Toggle"]->setToolTip(tr("Toggle playback"));

        _actions["Playback/Loop"] = new QAction(this);
        _actions["Playback/Loop"]->setCheckable(true);
        _actions["Playback/Loop"]->setText(tr("Loop Playback"));
        _actions["Playback/Once"] = new QAction(this);
        _actions["Playback/Once"]->setCheckable(true);
        _actions["Playback/Once"]->setText(tr("Playback Once"));
        _actions["Playback/PingPong"] = new QAction(this);
        _actions["Playback/PingPong"]->setCheckable(true);
        _actions["Playback/PingPong"]->setText(tr("Ping-Pong Playback"));
        _loopActionGroup = new QActionGroup(this);
        _loopActionGroup->setExclusive(true);
        _loopActionGroup->addAction(_actions["Playback/Loop"]);
        _loopActionGroup->addAction(_actions["Playback/Once"]);
        _loopActionGroup->addAction(_actions["Playback/PingPong"]);
        _actionToLoop[_actions["Playback/Loop"]] = timeline::Loop::Loop;
        _actionToLoop[_actions["Playback/Once"]] = timeline::Loop::Once;
        _actionToLoop[_actions["Playback/PingPong"]] = timeline::Loop::PingPong;
        _loopToActions[timeline::Loop::Loop] = _actions["Playback/Loop"];
        _loopToActions[timeline::Loop::Once] = _actions["Playback/Once"];
        _loopToActions[timeline::Loop::PingPong] = _actions["Playback/PingPong"];

        _actions["Playback/Start"] = new QAction(this);
        _actions["Playback/Start"]->setText(tr("Start Frame"));
        _actions["Playback/Start"]->setIcon(QIcon(":/Icons/FrameStart.svg"));
        _actions["Playback/Start"]->setShortcut(QKeySequence(Qt::Key_Home));
        _actions["Playback/End"] = new QAction(this);
        _actions["Playback/End"]->setText(tr("End Frame"));
        _actions["Playback/End"]->setIcon(QIcon(":/Icons/FrameEnd.svg"));
        _actions["Playback/End"]->setShortcut(QKeySequence(Qt::Key_End));
        _actions["Playback/FramePrev"] = new QAction(this);
        _actions["Playback/FramePrev"]->setText(tr("Previous Frame"));
        _actions["Playback/FramePrev"]->setIcon(QIcon(":/Icons/FramePrev.svg"));
        _actions["Playback/FramePrev"]->setShortcut(QKeySequence(Qt::Key_Left));
        _actions["Playback/FramePrevX10"] = new QAction(this);
        _actions["Playback/FramePrevX10"]->setText(tr("Previous Frame X10"));
        _actions["Playback/FramePrevX10"]->setShortcut(QKeySequence(Qt::SHIFT + Qt::Key_Left));
        _actions["Playback/FramePrevX100"] = new QAction(this);
        _actions["Playback/FramePrevX100"]->setText(tr("Previous Frame X100"));
        _actions["Playback/FramePrevX100"]->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_Left));
        _actions["Playback/FrameNext"] = new QAction(this);
        _actions["Playback/FrameNext"]->setText(tr("Next Frame"));
        _actions["Playback/FrameNext"]->setIcon(QIcon(":/Icons/FrameNext.svg"));
        _actions["Playback/FrameNext"]->setShortcut(QKeySequence(Qt::Key_Right));
        _actions["Playback/FrameNextX10"] = new QAction(this);
        _actions["Playback/FrameNextX10"]->setText(tr("Next Frame X10"));
        _actions["Playback/FrameNextX10"]->setShortcut(QKeySequence(Qt::SHIFT + Qt::Key_Right));
        _actions["Playback/FrameNextX100"] = new QAction(this);
        _actions["Playback/FrameNextX100"]->setText(tr("Next Frame X100"));
        _actions["Playback/FrameNextX100"]->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_Right));
        _actions["Playback/ClipPrev"] = new QAction(this);
        _actions["Playback/ClipPrev"]->setText(tr("Previous Clip"));
        _actions["Playback/ClipPrev"]->setShortcut(QKeySequence(Qt::Key_BracketLeft));
        _actions["Playback/ClipNext"] = new QAction(this);
        _actions["Playback/ClipNext"]->setText(tr("Next Clip"));
        _actions["Playback/ClipNext"]->setShortcut(QKeySequence(Qt::Key_BracketRight));

        _actions["Playback/SetInPoint"] = new QAction(this);
        _actions["Playback/SetInPoint"]->setText(tr("Set In Point"));
        _actions["Playback/SetInPoint"]->setIcon(QIcon(":/Icons/FrameStart.svg"));
        _actions["Playback/SetInPoint"]->setShortcut(QKeySequence(Qt::Key_I));
        _actions["Playback/ResetInPoint"] = new QAction(this);
        _actions["Playback/ResetInPoint"]->setText(tr("Reset In Point"));
        _actions["Playback/ResetInPoint"]->setIcon(QIcon(":/Icons/Reset.svg"));
        _actions["Playback/ResetInPoint"]->setShortcut(QKeySequence(Qt::SHIFT + Qt::Key_I));
        _actions["Playback/SetOutPoint"] = new QAction(this);
        _actions["Playback/SetOutPoint"]->setText(tr("Set Out Point"));
        _actions["Playback/SetOutPoint"]->setIcon(QIcon(":/Icons/FrameEnd.svg"));
        _actions["Playback/SetOutPoint"]->setShortcut(QKeySequence(Qt::Key_O));
        _actions["Playback/ResetOutPoint"] = new QAction(this);
        _actions["Playback/ResetOutPoint"]->setText(tr("Reset Out Point"));
        _actions["Playback/ResetOutPoint"]->setIcon(QIcon(":/Icons/Reset.svg"));
        _actions["Playback/ResetOutPoint"]->setShortcut(QKeySequence(Qt::SHIFT + Qt::Key_O));

        auto fileMenu = new QMenu;
        fileMenu->setTitle(tr("&File"));
        fileMenu->addAction(_actions["File/Open"]);
        fileMenu->addAction(_actions["File/Close"]);
        fileMenu->addSeparator();
        _recentFilesMenu = new QMenu;
        _recentFilesMenu->setTitle(tr("&Recent Files"));
        fileMenu->addMenu(_recentFilesMenu);
        fileMenu->addSeparator();
        fileMenu->addAction(_actions["File/Settings"]);
        fileMenu->addSeparator();
        fileMenu->addAction(_actions["File/Exit"]);

        auto playbackMenu = new QMenu;
        playbackMenu->setTitle(tr("&Playback"));
        playbackMenu->addAction(_actions["Playback/Stop"]);
        playbackMenu->addAction(_actions["Playback/Forward"]);
        playbackMenu->addAction(_actions["Playback/Reverse"]);
        playbackMenu->addAction(_actions["Playback/Toggle"]);
        playbackMenu->addSeparator();
        playbackMenu->addAction(_actions["Playback/Loop"]);
        playbackMenu->addAction(_actions["Playback/Once"]);
        playbackMenu->addAction(_actions["Playback/PingPong"]);
        playbackMenu->addSeparator();
        playbackMenu->addAction(_actions["Playback/Start"]);
        playbackMenu->addAction(_actions["Playback/End"]);
        playbackMenu->addAction(_actions["Playback/FramePrev"]);
        playbackMenu->addAction(_actions["Playback/FramePrevX10"]);
        playbackMenu->addAction(_actions["Playback/FramePrevX100"]);
        playbackMenu->addAction(_actions["Playback/FrameNext"]);
        playbackMenu->addAction(_actions["Playback/FrameNextX10"]);
        playbackMenu->addAction(_actions["Playback/FrameNextX100"]);
        playbackMenu->addAction(_actions["Playback/ClipPrev"]);
        playbackMenu->addAction(_actions["Playback/ClipNext"]);
        playbackMenu->addSeparator();
        playbackMenu->addAction(_actions["Playback/SetInPoint"]);
        playbackMenu->addAction(_actions["Playback/ResetInPoint"]);
        playbackMenu->addAction(_actions["Playback/SetOutPoint"]);
        playbackMenu->addAction(_actions["Playback/ResetOutPoint"]);

        auto menuBar = new QMenuBar;
        menuBar->addMenu(fileMenu);
        menuBar->addMenu(playbackMenu);
        setMenuBar(menuBar);

        _viewport = new qt::TimelineViewport;
        setCentralWidget(_viewport);

        _timelineWidget = new qt::TimelineWidget;
        _timelineWidget->setTimeObject(_timeObject);
        auto timelineDockWidget = new QDockWidget;
        timelineDockWidget->setObjectName("Timeline");
        timelineDockWidget->setWindowTitle(tr("Timeline"));
        timelineDockWidget->setFeatures(QDockWidget::NoDockWidgetFeatures);
        timelineDockWidget->setAllowedAreas(Qt::BottomDockWidgetArea);
        timelineDockWidget->setWidget(_timelineWidget);
        timelineDockWidget->setTitleBarWidget(new QWidget);
        addDockWidget(Qt::RightDockWidgetArea, timelineDockWidget);

        auto settingsWidget = new SettingsWidget(settingsObject, _timeObject);
        auto settingsDockWidget = new QDockWidget;
        settingsDockWidget->setObjectName("Settings");
        settingsDockWidget->setWindowTitle(tr("Settings"));
        settingsDockWidget->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);
        settingsDockWidget->setWidget(settingsWidget);
        settingsDockWidget->hide();
        addDockWidget(Qt::BottomDockWidgetArea, settingsDockWidget);

        _recentFilesUpdate();
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
            _recentFilesActionGroup,
            SIGNAL(triggered(QAction*)),
            SLOT(_recentFilesCallback(QAction*)));
        connect(
            _actions["File/Settings"],
            SIGNAL(triggered(bool)),
            settingsDockWidget,
            SLOT(setVisible(bool)));
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
            _actions["Playback/Reverse"],
            SIGNAL(triggered()),
            SLOT(_reverseCallback()));
        connect(
            _actions["Playback/Toggle"],
            SIGNAL(triggered()),
            SLOT(_togglePlaybackCallback()));
        connect(
            _actions["Playback/Start"],
            SIGNAL(triggered()),
            SLOT(_startCallback()));
        connect(
            _actions["Playback/End"],
            SIGNAL(triggered()),
            SLOT(_endCallback()));
        connect(
            _actions["Playback/FramePrev"],
            SIGNAL(triggered()),
            SLOT(_framePrevCallback()));
        connect(
            _actions["Playback/FramePrevX10"],
            SIGNAL(triggered()),
            SLOT(_framePrevX10Callback()));
        connect(
            _actions["Playback/FramePrevX100"],
            SIGNAL(triggered()),
            SLOT(_framePrevX100Callback()));
        connect(
            _actions["Playback/FrameNext"],
            SIGNAL(triggered()),
            SLOT(_frameNextCallback()));
        connect(
            _actions["Playback/FrameNextX10"],
            SIGNAL(triggered()),
            SLOT(_frameNextX10Callback()));
        connect(
            _actions["Playback/FrameNextX100"],
            SIGNAL(triggered()),
            SLOT(_frameNextX100Callback()));
        connect(
            _actions["Playback/ClipPrev"],
            SIGNAL(triggered()),
            SLOT(_clipPrevCallback()));
        connect(
            _actions["Playback/ClipNext"],
            SIGNAL(triggered()),
            SLOT(_clipNextCallback()));

        connect(
            _playbackActionGroup,
            SIGNAL(triggered(QAction*)),
            SLOT(_playbackCallback(QAction*)));

        connect(
            _loopActionGroup,
            SIGNAL(triggered(QAction*)),
            SLOT(_loopCallback(QAction*)));

        connect(
            settingsDockWidget,
            SIGNAL(visibilityChanged(bool)),
            SLOT(_settingsVisibleCallback(bool)));

        connect(
            _settingsObject,
            SIGNAL(recentFilesChanged(const QList<QString>&)),
            SLOT(_recentFilesCallback()));

        resize(640, 360);
        QSettings settings;
        restoreGeometry(settings.value("geometry").toByteArray());
        restoreState(settings.value("windowState").toByteArray());
    }

    void MainWindow::setTimeline(qt::TimelineObject* timeline)
    {
        if (timeline == _timeline)
            return;
        if (_timeline)
        {
            disconnect(
                _timeline,
                SIGNAL(playbackChanged(tlr::timeline::Playback)),
                this,
                SLOT(_playbackCallback(tlr::timeline::Playback)));
            disconnect(
                _timeline,
                SIGNAL(loopChanged(tlr::timeline::Loop)),
                this,
                SLOT(_loopCallback(tlr::timeline::Loop)));
            disconnect(
                _actions["Playback/SetInPoint"],
                SIGNAL(triggered(bool)),
                _timeline,
                SLOT(setInPoint()));
            disconnect(
                _actions["Playback/ResetInPoint"],
                SIGNAL(triggered(bool)),
                _timeline,
                SLOT(resetInPoint()));
            disconnect(
                _actions["Playback/SetOutPoint"],
                SIGNAL(triggered(bool)),
                _timeline,
                SLOT(setOutPoint()));
            disconnect(
                _actions["Playback/ResetOutPoint"],
                SIGNAL(triggered(bool)),
                _timeline,
                SLOT(resetOutPoint()));
        }
        _timeline = timeline;
        if (_timeline)
        {
            connect(
                _timeline,
                SIGNAL(playbackChanged(tlr::timeline::Playback)),
                SLOT(_playbackCallback(tlr::timeline::Playback)));
            connect(
                _timeline,
                SIGNAL(loopChanged(tlr::timeline::Loop)),
                SLOT(_loopCallback(tlr::timeline::Loop)));
            connect(
                _actions["Playback/SetInPoint"],
                SIGNAL(triggered(bool)),
                _timeline,
                SLOT(setInPoint()));
            connect(
                _actions["Playback/ResetInPoint"],
                SIGNAL(triggered(bool)),
                _timeline,
                SLOT(resetInPoint()));
            connect(
                _actions["Playback/SetOutPoint"],
                SIGNAL(triggered(bool)),
                _timeline,
                SLOT(setOutPoint()));
            connect(
                _actions["Playback/ResetOutPoint"],
                SIGNAL(triggered(bool)),
                _timeline,
                SLOT(resetOutPoint()));
        }
        _viewport->setTimeline(_timeline);
        _timelineWidget->setTimeline(_timeline);
        //_filmstripWidget->setFileName(_timeline ? _timeline->fileName() : std::string());
        _timelineUpdate();
    }

    void MainWindow::closeEvent(QCloseEvent* event)
    {
        QSettings settings;
        settings.setValue("geometry", saveGeometry());
        settings.setValue("windowState", saveState());
        QMainWindow::closeEvent(event);
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

    void MainWindow::_recentFilesCallback(QAction* action)
    {
        const auto i = _actionToRecentFile.find(action);
        if (i != _actionToRecentFile.end())
        {
            fileOpen(i.value());
        }
    }

    void MainWindow::_recentFilesCallback()
    {
        _recentFilesUpdate();
    }

    void MainWindow::_settingsVisibleCallback(bool value)
    {
        _actions["File/Settings"]->setChecked(value);
    }

    void MainWindow::_playbackCallback(QAction* action)
    {
        const auto i = _actionToPlayback.find(action);
        if (i != _actionToPlayback.end())
        {
            _timeline->setPlayback(i.value());
        }
    }

    void MainWindow::_playbackCallback(tlr::timeline::Playback value)
    {
        const QSignalBlocker blocker(_playbackActionGroup);
        const auto i = _playbackToActions.find(value);
        if (i != _playbackToActions.end())
        {
            i.value()->setChecked(true);
        }
    }

    void MainWindow::_loopCallback(QAction* action)
    {
        const auto i = _actionToLoop.find(action);
        if (i != _actionToLoop.end())
        {
            _timeline->setLoop(i.value());
        }
    }

    void MainWindow::_loopCallback(tlr::timeline::Loop value)
    {
        const QSignalBlocker blocker(_loopActionGroup);
        const auto i = _loopToActions.find(value);
        if (i != _loopToActions.end())
        {
            i.value()->setChecked(true);
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

    void MainWindow::_reverseCallback()
    {
        if (_timeline)
        {
            _timeline->reverse();
        }
    }

    void MainWindow::_togglePlaybackCallback()
    {
        if (_timeline)
        {
            _timeline->togglePlayback();
        }
    }

    void MainWindow::_startCallback()
    {
        if (_timeline)
        {
            _timeline->start();
        }
    }

    void MainWindow::_endCallback()
    {
        if (_timeline)
        {
            _timeline->end();
        }
    }

    void MainWindow::_framePrevCallback()
    {
        if (_timeline)
        {
            _timeline->framePrev();
        }
    }

    void MainWindow::_framePrevX10Callback()
    {
        if (_timeline)
        {
            _timeline->timeAction(timeline::TimeAction::FramePrevX10);
        }
    }

    void MainWindow::_framePrevX100Callback()
    {
        if (_timeline)
        {
            _timeline->timeAction(timeline::TimeAction::FramePrevX100);
        }
    }

    void MainWindow::_frameNextCallback()
    {
        if (_timeline)
        {
            _timeline->frameNext();
        }
    }

    void MainWindow::_frameNextX10Callback()
    {
        if (_timeline)
        {
            _timeline->timeAction(timeline::TimeAction::FrameNextX10);
        }
    }

    void MainWindow::_frameNextX100Callback()
    {
        if (_timeline)
        {
            _timeline->timeAction(timeline::TimeAction::FrameNextX100);
        }
    }

    void MainWindow::_clipPrevCallback()
    {
        if (_timeline)
        {
            _timeline->clipPrev();
        }
    }

    void MainWindow::_clipNextCallback()
    {
        if (_timeline)
        {
            _timeline->clipNext();
        }
    }

    void MainWindow::_recentFilesUpdate()
    {
        for (const auto& i : _actionToRecentFile.keys())
        {
            _recentFilesActionGroup->removeAction(i);
            i->setParent(nullptr);
            delete i;
        }
        _actionToRecentFile.clear();
        _recentFilesMenu->clear();
        const auto& recentFiles = _settingsObject->recentFiles();
        for (size_t i = 0; i < recentFiles.size(); ++i)
        {
            auto action = new QAction;
            const auto& file = recentFiles[i];
            action->setText(QString("%1 %2").arg(i + 1).arg(file));
            _recentFilesActionGroup->addAction(action);
            _actionToRecentFile[action] = file;
            _recentFilesMenu->addAction(action);
        }
    }

    void MainWindow::_playbackUpdate()
    {
        timeline::Playback playback = timeline::Playback::Stop;
        if (_timeline)
        {
            playback = _timeline->playback();
        }
        _actions["Playback/Stop"]->setChecked(timeline::Playback::Stop == playback);
        _actions["Playback/Forward"]->setChecked(timeline::Playback::Forward == playback);
        _actions["Playback/Reverse"]->setChecked(timeline::Playback::Reverse == playback);
    }

    void MainWindow::_timelineUpdate()
    {
        if (_timeline)
        {
            _actions["File/Close"]->setEnabled(true);

            _actions["Playback/Stop"]->setEnabled(true);
            _actions["Playback/Forward"]->setEnabled(true);
            _actions["Playback/Reverse"]->setEnabled(true);
            const auto playbackAction = _playbackToActions.find(_timeline->playback());
            if (playbackAction != _playbackToActions.end())
            {
                playbackAction.value()->setChecked(true);
            }
            _actions["Playback/Toggle"]->setEnabled(true);

            _actions["Playback/Loop"]->setEnabled(true);
            _actions["Playback/Once"]->setEnabled(true);
            _actions["Playback/PingPong"]->setEnabled(true);
            const auto loopAction = _loopToActions.find(_timeline->loop());
            if (loopAction != _loopToActions.end())
            {
                loopAction.value()->setChecked(true);
            }

            _actions["Playback/Start"]->setEnabled(true);
            _actions["Playback/End"]->setEnabled(true);
            _actions["Playback/FramePrev"]->setEnabled(true);
            _actions["Playback/FramePrevX10"]->setEnabled(true);
            _actions["Playback/FramePrevX100"]->setEnabled(true);
            _actions["Playback/FrameNext"]->setEnabled(true);
            _actions["Playback/FrameNextX10"]->setEnabled(true);
            _actions["Playback/FrameNextX100"]->setEnabled(true);

            _actions["Playback/SetInPoint"]->setEnabled(true);
            _actions["Playback/ResetInPoint"]->setEnabled(true);
            _actions["Playback/SetOutPoint"]->setEnabled(true);
            _actions["Playback/ResetOutPoint"]->setEnabled(true);
        }
        else
        {
            _actions["File/Close"]->setEnabled(false);

            _actions["Playback/Stop"]->setEnabled(false);
            _actions["Playback/Stop"]->setChecked(false);
            _actions["Playback/Forward"]->setEnabled(false);
            _actions["Playback/Forward"]->setChecked(false);
            _actions["Playback/Reverse"]->setEnabled(false);
            _actions["Playback/Reverse"]->setChecked(false);
            _actions["Playback/Toggle"]->setEnabled(false);

            _actions["Playback/Loop"]->setEnabled(false);
            _actions["Playback/Loop"]->setChecked(false);
            _actions["Playback/Once"]->setEnabled(false);
            _actions["Playback/Once"]->setChecked(false);
            _actions["Playback/PingPong"]->setEnabled(false);
            _actions["Playback/PingPong"]->setChecked(false);

            _actions["Playback/Start"]->setEnabled(false);
            _actions["Playback/End"]->setEnabled(false);
            _actions["Playback/FramePrev"]->setEnabled(false);
            _actions["Playback/FramePrevX10"]->setEnabled(false);
            _actions["Playback/FramePrevX100"]->setEnabled(false);
            _actions["Playback/FrameNext"]->setEnabled(false);
            _actions["Playback/FrameNextX10"]->setEnabled(false);
            _actions["Playback/FrameNextX100"]->setEnabled(false);

            _actions["Playback/SetInPoint"]->setEnabled(false);
            _actions["Playback/ResetInPoint"]->setEnabled(false);
            _actions["Playback/SetOutPoint"]->setEnabled(false);
            _actions["Playback/ResetOutPoint"]->setEnabled(false);
        }
    }
}
