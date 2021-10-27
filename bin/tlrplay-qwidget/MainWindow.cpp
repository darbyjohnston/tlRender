// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021 Darby Johnston
// All rights reserved.

#include "MainWindow.h"

#include "App.h"
#include "ImageOptionsWidget.h"
#include "SettingsWidget.h"

#include <tlrCore/File.h>
#include <tlrCore/String.h>
#include <tlrCore/StringFormat.h>

#include <QDockWidget>
#include <QDragEnterEvent>
#include <QDragMoveEvent>
#include <QFileDialog>
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
        const std::shared_ptr<core::Context>& context,
        QWidget* parent) :
        QMainWindow(parent),
        _settingsObject(settingsObject),
        _timeObject(timeObject)
    {
        setFocusPolicy(Qt::ClickFocus);
        setAcceptDrops(true);

        _context = context;

        _actions["File/Open"] = new QAction(this);
        _actions["File/Open"]->setText(tr("Open"));
        _actions["File/Open"]->setShortcut(QKeySequence::Open);
        _actions["File/Close"] = new QAction(this);
        _actions["File/Close"]->setText(tr("Close"));
        _actions["File/Close"]->setShortcut(QKeySequence::Close);
        _actions["File/CloseAll"] = new QAction(this);
        _actions["File/CloseAll"]->setText(tr("Close All"));
        _actions["File/Next"] = new QAction(this);
        _actions["File/Next"]->setText(tr("Next"));
        _actions["File/Next"]->setShortcut(QKeySequence::MoveToNextPage);
        _actions["File/Prev"] = new QAction(this);
        _actions["File/Prev"]->setText(tr("Previous"));
        _actions["File/Prev"]->setShortcut(QKeySequence::MoveToPreviousPage);
        _recentFilesActionGroup = new QActionGroup(this);
        _layersActionGroup = new QActionGroup(this);
        _layersActionGroup->setExclusive(true);
        _actions["File/Exit"] = new QAction(this);
        _actions["File/Exit"]->setText(tr("Exit"));
        _actions["File/Exit"]->setShortcut(QKeySequence::Quit);

        _actions["Window/Resize1280x720"] = new QAction(this);
        _actions["Window/Resize1280x720"]->setText(tr("Resize 1280x720"));
        _actions["Window/Resize1920x1080"] = new QAction(this);
        _actions["Window/Resize1920x1080"]->setText(tr("Resize 1920x1080"));
        _actions["Window/FullScreen"] = new QAction(this);
        _actions["Window/FullScreen"]->setText(tr("Toggle Full Screen"));
        _actions["Window/FullScreen"]->setShortcut(QKeySequence(Qt::Key_U));
        _actions["Window/Secondary"] = new QAction(this);
        _actions["Window/Secondary"]->setCheckable(true);
        _actions["Window/Secondary"]->setText(tr("Secondary Window"));
        _actions["Window/Secondary"]->setShortcut(QKeySequence(Qt::Key_Y));

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

        _actions["Time/Start"] = new QAction(this);
        _actions["Time/Start"]->setText(tr("Start"));
        _actions["Time/Start"]->setIcon(QIcon(":/Icons/TimeStart.svg"));
        _actions["Time/Start"]->setShortcut(QKeySequence(Qt::Key_Home));
        _actions["Time/End"] = new QAction(this);
        _actions["Time/End"]->setText(tr("End"));
        _actions["Time/End"]->setIcon(QIcon(":/Icons/TimeEnd.svg"));
        _actions["Time/End"]->setShortcut(QKeySequence(Qt::Key_End));
        _actions["Time/FramePrev"] = new QAction(this);
        _actions["Time/FramePrev"]->setText(tr("Previous Frame"));
        _actions["Time/FramePrev"]->setIcon(QIcon(":/Icons/FramePrev.svg"));
        _actions["Time/FramePrev"]->setShortcut(QKeySequence(Qt::Key_Left));
        _actions["Time/FramePrevX10"] = new QAction(this);
        _actions["Time/FramePrevX10"]->setText(tr("Previous Frame X10"));
        _actions["Time/FramePrevX10"]->setShortcut(QKeySequence(Qt::SHIFT + Qt::Key_Left));
        _actions["Time/FramePrevX100"] = new QAction(this);
        _actions["Time/FramePrevX100"]->setText(tr("Previous Frame X100"));
        _actions["Time/FramePrevX100"]->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_Left));
        _actions["Time/FrameNext"] = new QAction(this);
        _actions["Time/FrameNext"]->setText(tr("Next Frame"));
        _actions["Time/FrameNext"]->setIcon(QIcon(":/Icons/FrameNext.svg"));
        _actions["Time/FrameNext"]->setShortcut(QKeySequence(Qt::Key_Right));
        _actions["Time/FrameNextX10"] = new QAction(this);
        _actions["Time/FrameNextX10"]->setText(tr("Next Frame X10"));
        _actions["Time/FrameNextX10"]->setShortcut(QKeySequence(Qt::SHIFT + Qt::Key_Right));
        _actions["Time/FrameNextX100"] = new QAction(this);
        _actions["Time/FrameNextX100"]->setText(tr("Next Frame X100"));
        _actions["Time/FrameNextX100"]->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_Right));

        _actions["InOutPoints/SetInPoint"] = new QAction(this);
        _actions["InOutPoints/SetInPoint"]->setText(tr("Set In Point"));
        _actions["InOutPoints/SetInPoint"]->setIcon(QIcon(":/Icons/TimeStart.svg"));
        _actions["InOutPoints/SetInPoint"]->setShortcut(QKeySequence(Qt::Key_I));
        _actions["InOutPoints/ResetInPoint"] = new QAction(this);
        _actions["InOutPoints/ResetInPoint"]->setText(tr("Reset In Point"));
        _actions["InOutPoints/ResetInPoint"]->setIcon(QIcon(":/Icons/Reset.svg"));
        _actions["InOutPoints/ResetInPoint"]->setShortcut(QKeySequence(Qt::SHIFT + Qt::Key_I));
        _actions["InOutPoints/SetOutPoint"] = new QAction(this);
        _actions["InOutPoints/SetOutPoint"]->setText(tr("Set Out Point"));
        _actions["InOutPoints/SetOutPoint"]->setIcon(QIcon(":/Icons/TimeEnd.svg"));
        _actions["InOutPoints/SetOutPoint"]->setShortcut(QKeySequence(Qt::Key_O));
        _actions["InOutPoints/ResetOutPoint"] = new QAction(this);
        _actions["InOutPoints/ResetOutPoint"]->setText(tr("Reset Out Point"));
        _actions["InOutPoints/ResetOutPoint"]->setIcon(QIcon(":/Icons/Reset.svg"));
        _actions["InOutPoints/ResetOutPoint"]->setShortcut(QKeySequence(Qt::SHIFT + Qt::Key_O));

        _actions["Tools/ImageOptions"] = new QAction(this);
        _actions["Tools/ImageOptions"]->setCheckable(true);
        _actions["Tools/ImageOptions"]->setText(tr("Image Options"));
        _actions["Tools/Settings"] = new QAction(this);
        _actions["Tools/Settings"]->setCheckable(true);
        _actions["Tools/Settings"]->setText(tr("Settings"));

        auto fileMenu = new QMenu;
        fileMenu->setTitle(tr("&File"));
        fileMenu->addAction(_actions["File/Open"]);
        fileMenu->addAction(_actions["File/Close"]);
        fileMenu->addAction(_actions["File/CloseAll"]);
        fileMenu->addSeparator();
        _recentFilesMenu = new QMenu;
        _recentFilesMenu->setTitle(tr("&Recent Files"));
        fileMenu->addMenu(_recentFilesMenu);
        fileMenu->addSeparator();
        fileMenu->addAction(_actions["File/Next"]);
        fileMenu->addAction(_actions["File/Prev"]);
        fileMenu->addSeparator();
        _layersMenu = new QMenu;
        _layersMenu->setTitle(tr("&Layers"));
        fileMenu->addMenu(_layersMenu);
        fileMenu->addSeparator();
        fileMenu->addAction(_actions["File/Exit"]);

        auto windowMenu = new QMenu;
        windowMenu->setTitle(tr("&Window"));
        windowMenu->addAction(_actions["Window/Resize1280x720"]);
        windowMenu->addAction(_actions["Window/Resize1920x1080"]);
        windowMenu->addSeparator();
        windowMenu->addAction(_actions["Window/FullScreen"]);
        windowMenu->addAction(_actions["Window/Secondary"]);

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

        auto timeMenu = new QMenu;
        timeMenu->setTitle(tr("&Time"));
        timeMenu->addAction(_actions["Time/Start"]);
        timeMenu->addAction(_actions["Time/End"]);
        timeMenu->addSeparator();
        timeMenu->addAction(_actions["Time/FramePrev"]);
        timeMenu->addAction(_actions["Time/FramePrevX10"]);
        timeMenu->addAction(_actions["Time/FramePrevX100"]);
        timeMenu->addAction(_actions["Time/FrameNext"]);
        timeMenu->addAction(_actions["Time/FrameNextX10"]);
        timeMenu->addAction(_actions["Time/FrameNextX100"]);

        auto inOutPointsMenu = new QMenu;
        inOutPointsMenu->setTitle(tr("&In/Out Points"));
        inOutPointsMenu->addAction(_actions["InOutPoints/SetInPoint"]);
        inOutPointsMenu->addAction(_actions["InOutPoints/ResetInPoint"]);
        inOutPointsMenu->addAction(_actions["InOutPoints/SetOutPoint"]);
        inOutPointsMenu->addAction(_actions["InOutPoints/ResetOutPoint"]);

        auto toolsMenu = new QMenu;
        toolsMenu->setTitle(tr("&Tools"));
        toolsMenu->addAction(_actions["Tools/ImageOptions"]);
        toolsMenu->addAction(_actions["Tools/Settings"]);

        auto menuBar = new QMenuBar;
        menuBar->addMenu(fileMenu);
        menuBar->addMenu(windowMenu);
        menuBar->addMenu(playbackMenu);
        menuBar->addMenu(timeMenu);
        menuBar->addMenu(inOutPointsMenu);
        menuBar->addMenu(toolsMenu);
        setMenuBar(menuBar);

        _tabWidget = new QTabWidget;
        _tabWidget->setTabsClosable(true);
        setCentralWidget(_tabWidget);

        auto imageOptionsWidget = new ImageOptionsWidget();
        auto imageOptionsDockWidget = new QDockWidget;
        imageOptionsDockWidget->setObjectName("ImageOptions");
        imageOptionsDockWidget->setWindowTitle(tr("Image Options"));
        imageOptionsDockWidget->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);
        imageOptionsDockWidget->setWidget(imageOptionsWidget);
        imageOptionsDockWidget->hide();
        addDockWidget(Qt::RightDockWidgetArea, imageOptionsDockWidget);

        auto settingsWidget = new SettingsWidget(settingsObject, _timeObject);
        auto settingsDockWidget = new QDockWidget;
        settingsDockWidget->setObjectName("Settings");
        settingsDockWidget->setWindowTitle(tr("Settings"));
        settingsDockWidget->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);
        settingsDockWidget->setWidget(settingsWidget);
        settingsDockWidget->hide();
        addDockWidget(Qt::RightDockWidgetArea, settingsDockWidget);

        _recentFilesUpdate();
        _layersUpdate();
        _playbackUpdate();
        _timelineUpdate();

        connect(
            _actions["File/Open"],
            SIGNAL(triggered()),
            SLOT(_openCallback()));
        connect(
            _actions["File/Close"],
            SIGNAL(triggered()),
            SLOT(_closeCallback()));
        connect(
            _actions["File/CloseAll"],
            SIGNAL(triggered()),
            SLOT(_closeAllCallback()));
        connect(
            _recentFilesActionGroup,
            SIGNAL(triggered(QAction*)),
            SLOT(_recentFilesCallback(QAction*)));
        connect(
            _actions["File/Next"],
            SIGNAL(triggered()),
            SLOT(_nextCallback()));
        connect(
            _actions["File/Prev"],
            SIGNAL(triggered()),
            SLOT(_prevCallback()));
        connect(
            _layersActionGroup,
            SIGNAL(triggered(QAction*)),
            SLOT(_layersCallback(QAction*)));
        connect(
            _actions["File/Exit"],
            SIGNAL(triggered()),
            qApp,
            SLOT(quit()));

        connect(
            _actions["Window/Resize1280x720"],
            SIGNAL(triggered()),
            SLOT(_resize1280x720Callback()));
        connect(
            _actions["Window/Resize1920x1080"],
            SIGNAL(triggered()),
            SLOT(_resize1920x1080Callback()));
        connect(
            _actions["Window/FullScreen"],
            SIGNAL(triggered()),
            SLOT(_fullScreenCallback()));
        connect(
            _actions["Window/Secondary"],
            SIGNAL(toggled(bool)),
            SLOT(_secondaryWindowCallback(bool)));

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
            _actions["Time/Start"],
            SIGNAL(triggered()),
            SLOT(_startCallback()));
        connect(
            _actions["Time/End"],
            SIGNAL(triggered()),
            SLOT(_endCallback()));
        connect(
            _actions["Time/FramePrev"],
            SIGNAL(triggered()),
            SLOT(_framePrevCallback()));
        connect(
            _actions["Time/FramePrevX10"],
            SIGNAL(triggered()),
            SLOT(_framePrevX10Callback()));
        connect(
            _actions["Time/FramePrevX100"],
            SIGNAL(triggered()),
            SLOT(_framePrevX100Callback()));
        connect(
            _actions["Time/FrameNext"],
            SIGNAL(triggered()),
            SLOT(_frameNextCallback()));
        connect(
            _actions["Time/FrameNextX10"],
            SIGNAL(triggered()),
            SLOT(_frameNextX10Callback()));
        connect(
            _actions["Time/FrameNextX100"],
            SIGNAL(triggered()),
            SLOT(_frameNextX100Callback()));

        connect(
            _actions["Tools/ImageOptions"],
            SIGNAL(triggered(bool)),
            imageOptionsDockWidget,
            SLOT(setVisible(bool)));
        connect(
            _actions["Tools/Settings"],
            SIGNAL(triggered(bool)),
            settingsDockWidget,
            SLOT(setVisible(bool)));

        connect(
            _playbackActionGroup,
            SIGNAL(triggered(QAction*)),
            SLOT(_playbackCallback(QAction*)));

        connect(
            _loopActionGroup,
            SIGNAL(triggered(QAction*)),
            SLOT(_loopCallback(QAction*)));

        connect(
            _tabWidget,
            SIGNAL(currentChanged(int)),
            SLOT(_currentTabCallback(int)));
        connect(
            _tabWidget,
            SIGNAL(tabCloseRequested(int)),
            SLOT(_closeTabCallback(int)));

        connect(
            imageOptionsWidget,
            SIGNAL(imageOptionsChanged(const tlr::gl::ImageOptions&)),
            SLOT(_imageOptionsCallback(const tlr::gl::ImageOptions&)));
        connect(
            imageOptionsDockWidget,
            SIGNAL(visibilityChanged(bool)),
            SLOT(_imageOptionsVisibleCallback(bool)));

        connect(
            settingsDockWidget,
            SIGNAL(visibilityChanged(bool)),
            SLOT(_settingsVisibleCallback(bool)));

        connect(
            _settingsObject,
            SIGNAL(recentFilesChanged(const QList<QString>&)),
            SLOT(_recentFilesCallback()));

        if (auto app = qobject_cast<App*>(qApp))
        {
            connect(
                app,
                SIGNAL(opened(tlr::qt::TimelinePlayer*)),
                SLOT(_openedCallback(tlr::qt::TimelinePlayer*)));
            connect(
                app,
                SIGNAL(closed(tlr::qt::TimelinePlayer*)),
                SLOT(_closedCallback(tlr::qt::TimelinePlayer*)));
            connect(
                app,
                SIGNAL(aboutToQuit()),
                SLOT(_saveSettingsCallback()));
        }

        resize(1280, 720);
        QSettings settings;
        auto ba = settings.value("geometry").toByteArray();
        if (!ba.isEmpty())
        {
            restoreGeometry(settings.value("geometry").toByteArray());
        }
        ba = settings.value("geometry").toByteArray();
        if (!ba.isEmpty())
        {
            restoreState(settings.value("windowState").toByteArray());
        }
    }

    MainWindow::~MainWindow()
    {
        if (_secondaryWindow)
        {
            delete _secondaryWindow;
            _secondaryWindow = nullptr;
        }
    }

    void MainWindow::setColorConfig(const gl::ColorConfig& colorConfig)
    {
        if (colorConfig != _colorConfig)
            return;
        _colorConfig = colorConfig;
        if (_secondaryWindow)
        {
            _secondaryWindow->setColorConfig(_colorConfig);
        }
    }

    void MainWindow::closeEvent(QCloseEvent* event)
    {
        _saveSettingsCallback();
        if (_secondaryWindow)
        {
            delete _secondaryWindow;
            _secondaryWindow = nullptr;
        }
        QMainWindow::closeEvent(event);
    }

    void MainWindow::dragEnterEvent(QDragEnterEvent* event)
    {
        const QMimeData* mimeData = event->mimeData();
        if (mimeData->hasUrls())
        {
            event->acceptProposedAction();
        }
    }

    void MainWindow::dragMoveEvent(QDragMoveEvent* event)
    {
        const QMimeData* mimeData = event->mimeData();
        if (mimeData->hasUrls())
        {
            event->acceptProposedAction();
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
            for (int i = 0; i < urlList.size(); ++i)
            {
                if (auto app = qobject_cast<App*>(qApp))
                {
                    app->open(urlList[i].toLocalFile().toUtf8());
                }
            }
        }
    }

    void MainWindow::_openCallback()
    {
        std::vector<std::string> extensions;
        for (const auto& i : timeline::getExtensions())
        {
            extensions.push_back("*" + i);
        }

        QString dir;
        if (_currentTimelinePlayer)
        {
            dir = QString::fromUtf8(_currentTimelinePlayer->path().get().c_str());
        }

        const auto fileName = QFileDialog::getOpenFileName(
            this,
            tr("Open Timeline"),
            dir,
            tr("Timeline Files") + " (" + QString::fromUtf8(string::join(extensions, ", ").c_str()) + ")");
        if (!fileName.isEmpty())
        {
            if (auto app = qobject_cast<App*>(qApp))
            {
                app->open(fileName);
            }
        }
    }

    void MainWindow::_openedCallback(qt::TimelinePlayer* timelinePlayer)
    {
        if (auto context = _context.lock())
        {
            auto widget = new qwidget::TimelineWidget(context);
            widget->setTimeObject(_timeObject);
            widget->setColorConfig(_colorConfig);
            widget->setTimelinePlayer(timelinePlayer);
            const file::Path& path = timelinePlayer->path();
            const int tab = _tabWidget->addTab(widget, QString::fromUtf8(path.get(-1, false).c_str()));
            const auto& videoInfo = timelinePlayer->videoInfo();
            const std::string toolTip = string::Format("{0}\n{1}").arg(path.get()).arg(!videoInfo.empty() ? videoInfo[0] : imaging::Info());
            _tabWidget->setTabToolTip(tab, QString::fromUtf8(toolTip.c_str()));
            _timelinePlayers.append(timelinePlayer);
            _timelineWidgets.append(widget);
            _setCurrentTimeline(timelinePlayer);
        }
    }

    void MainWindow::_closeCallback()
    {
        if (auto app = qobject_cast<App*>(qApp))
        {
            app->close(_currentTimelinePlayer);
        }
    }

    void MainWindow::_closeAllCallback()
    {
        if (auto app = qobject_cast<App*>(qApp))
        {
            app->closeAll();
        }
    }

    void MainWindow::_closedCallback(qt::TimelinePlayer* timelinePlayer)
    {
        int i = _timelinePlayers.indexOf(timelinePlayer);
        if (i != -1)
        {
            auto widget = _tabWidget->widget(i);
            _tabWidget->removeTab(i);
            delete widget;
            _timelinePlayers.removeAt(i);
            _timelineWidgets.removeAt(i);
            if (timelinePlayer == _currentTimelinePlayer)
            {
                if (i > _timelinePlayers.size())
                {
                    --i;
                }
                _setCurrentTimeline(i >= 0 && i < _timelinePlayers.size() ? _timelinePlayers[i] : nullptr);
            }
            _timelineUpdate();
        }
    }

    void MainWindow::_recentFilesCallback(QAction* action)
    {
        const auto i = _actionToRecentFile.find(action);
        if (i != _actionToRecentFile.end())
        {
            if (auto app = qobject_cast<App*>(qApp))
            {
                app->open(i.value());
            }
        }
    }

    void MainWindow::_recentFilesCallback()
    {
        _recentFilesUpdate();
    }

    void MainWindow::_nextCallback()
    {
        if (_timelinePlayers.size() > 1)
        {
            int i = _timelinePlayers.indexOf(_currentTimelinePlayer);
            ++i;
            if (i >= _timelinePlayers.size())
            {
                i = 0;
            }
            _setCurrentTimeline(_timelinePlayers[i]);
        }
    }

    void MainWindow::_prevCallback()
    {
        if (_timelinePlayers.size() > 1)
        {
            int i = _timelinePlayers.indexOf(_currentTimelinePlayer);
            --i;
            if (i < 0)
            {
                i = _timelinePlayers.size() - 1;
            }
            _setCurrentTimeline(_timelinePlayers[i]);
        }
    }

    void MainWindow::_layersCallback(QAction* action)
    {
        if (_currentTimelinePlayer)
        {
            const auto i = _actionToLayer.find(action);
            if (i != _actionToLayer.end())
            {
                _currentTimelinePlayer->setVideoLayer(i.value());
            }
        }
    }

    void MainWindow::_layersCallback(int value)
    {
        const auto& actions = _layersActionGroup->actions();
        if (value >= 0 && value < actions.size())
        {
            actions[value]->setChecked(true);
        }
    }

    void MainWindow::_resize1280x720Callback()
    {
        resize(1280, 720);
    }

    void MainWindow::_resize1920x1080Callback()
    {
        resize(1920, 1080);
    }

    void MainWindow::_fullScreenCallback()
    {
        setWindowState(windowState() ^ Qt::WindowFullScreen);
    }

    void MainWindow::_secondaryWindowCallback(bool value)
    {
        if (value && !_secondaryWindow)
        {
            if (auto context = _context.lock())
            {
                _secondaryWindow = new SecondaryWindow(context);
                _secondaryWindow->setColorConfig(_colorConfig);
                _secondaryWindow->setTimelinePlayer(_currentTimelinePlayer);

                connect(
                    _secondaryWindow,
                    SIGNAL(destroyed(QObject*)),
                    SLOT(_secondaryWindowDestroyedCallback()));

                _secondaryWindow->resize(1280, 720);
                _secondaryWindow->show();
            }
        }
        else if (!value && _secondaryWindow)
        {
            delete _secondaryWindow;
            _secondaryWindow = nullptr;
        }
    }

    void MainWindow::_secondaryWindowDestroyedCallback()
    {
        _secondaryWindow = nullptr;
        _actions["Window/Secondary"]->setChecked(false);
    }

    void MainWindow::_currentTabCallback(int index)
    {
        if (index >= 0 && index < _timelinePlayers.size())
        {
            _setCurrentTimeline(_timelinePlayers[index]);
        }
    }

    void MainWindow::_closeTabCallback(int index)
    {
        if (auto app = qobject_cast<App*>(qApp))
        {
            if (index >= 0 && index < _timelinePlayers.size())
            {
                app->close(_timelinePlayers[index]);
            }
        }
    }

    void MainWindow::_playbackCallback(QAction* action)
    {
        if (_currentTimelinePlayer)
        {
            const auto i = _actionToPlayback.find(action);
            if (i != _actionToPlayback.end())
            {
                _currentTimelinePlayer->setPlayback(i.value());
            }
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
        if (_currentTimelinePlayer)
        {
            const auto i = _actionToLoop.find(action);
            if (i != _actionToLoop.end())
            {
                _currentTimelinePlayer->setLoop(i.value());
            }
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
        if (_currentTimelinePlayer)
        {
            _currentTimelinePlayer->stop();
        }
    }

    void MainWindow::_forwardCallback()
    {
        if (_currentTimelinePlayer)
        {
            _currentTimelinePlayer->forward();
        }
    }

    void MainWindow::_reverseCallback()
    {
        if (_currentTimelinePlayer)
        {
            _currentTimelinePlayer->reverse();
        }
    }

    void MainWindow::_togglePlaybackCallback()
    {
        if (_currentTimelinePlayer)
        {
            _currentTimelinePlayer->togglePlayback();
        }
    }

    void MainWindow::_startCallback()
    {
        if (_currentTimelinePlayer)
        {
            _currentTimelinePlayer->start();
        }
    }

    void MainWindow::_endCallback()
    {
        if (_currentTimelinePlayer)
        {
            _currentTimelinePlayer->end();
        }
    }

    void MainWindow::_framePrevCallback()
    {
        if (_currentTimelinePlayer)
        {
            _currentTimelinePlayer->framePrev();
        }
    }

    void MainWindow::_framePrevX10Callback()
    {
        if (_currentTimelinePlayer)
        {
            _currentTimelinePlayer->timeAction(timeline::TimeAction::FramePrevX10);
        }
    }

    void MainWindow::_framePrevX100Callback()
    {
        if (_currentTimelinePlayer)
        {
            _currentTimelinePlayer->timeAction(timeline::TimeAction::FramePrevX100);
        }
    }

    void MainWindow::_frameNextCallback()
    {
        if (_currentTimelinePlayer)
        {
            _currentTimelinePlayer->frameNext();
        }
    }

    void MainWindow::_frameNextX10Callback()
    {
        if (_currentTimelinePlayer)
        {
            _currentTimelinePlayer->timeAction(timeline::TimeAction::FrameNextX10);
        }
    }

    void MainWindow::_frameNextX100Callback()
    {
        if (_currentTimelinePlayer)
        {
            _currentTimelinePlayer->timeAction(timeline::TimeAction::FrameNextX100);
        }
    }

    void MainWindow::_imageOptionsCallback(const tlr::gl::ImageOptions& value)
    {
        for (int i = 0; i < _timelineWidgets.count(); ++i)
        {
            _timelineWidgets[i]->setImageOptions(value);
        }
    }

    void MainWindow::_imageOptionsVisibleCallback(bool value)
    {
        _actions["Tools/ImageOptions"]->setChecked(value);
    }

    void MainWindow::_settingsVisibleCallback(bool value)
    {
        _actions["Tools/Settings"]->setChecked(value);
    }

    void MainWindow::_saveSettingsCallback()
    {
        QSettings settings;
        settings.setValue("geometry", saveGeometry());
        settings.setValue("windowState", saveState());
    }

    void MainWindow::_setCurrentTimeline(qt::TimelinePlayer* timelinePlayer)
    {
        if (timelinePlayer == _currentTimelinePlayer)
            return;
        if (_currentTimelinePlayer)
        {
            disconnect(
                _currentTimelinePlayer,
                SIGNAL(playbackChanged(tlr::timeline::Playback)),
                this,
                SLOT(_playbackCallback(tlr::timeline::Playback)));
            disconnect(
                _currentTimelinePlayer,
                SIGNAL(loopChanged(tlr::timeline::Loop)),
                this,
                SLOT(_loopCallback(tlr::timeline::Loop)));
            disconnect(
                _currentTimelinePlayer,
                SIGNAL(videoLayerChanged(int)),
                this,
                SLOT(_layersCallback(int)));
            disconnect(
                _actions["InOutPoints/SetInPoint"],
                SIGNAL(triggered(bool)),
                _currentTimelinePlayer,
                SLOT(setInPoint()));
            disconnect(
                _actions["InOutPoints/ResetInPoint"],
                SIGNAL(triggered(bool)),
                _currentTimelinePlayer,
                SLOT(resetInPoint()));
            disconnect(
                _actions["InOutPoints/SetOutPoint"],
                SIGNAL(triggered(bool)),
                _currentTimelinePlayer,
                SLOT(setOutPoint()));
            disconnect(
                _actions["InOutPoints/ResetOutPoint"],
                SIGNAL(triggered(bool)),
                _currentTimelinePlayer,
                SLOT(resetOutPoint()));
        }
        _currentTimelinePlayer = timelinePlayer;
        if (_currentTimelinePlayer)
        {
            connect(
                _currentTimelinePlayer,
                SIGNAL(playbackChanged(tlr::timeline::Playback)),
                SLOT(_playbackCallback(tlr::timeline::Playback)));
            connect(
                _currentTimelinePlayer,
                SIGNAL(loopChanged(tlr::timeline::Loop)),
                SLOT(_loopCallback(tlr::timeline::Loop)));
            connect(
                _currentTimelinePlayer,
                SIGNAL(videoLayerChanged(int)),
                SLOT(_layersCallback(int)));
            connect(
                _actions["InOutPoints/SetInPoint"],
                SIGNAL(triggered(bool)),
                _currentTimelinePlayer,
                SLOT(setInPoint()));
            connect(
                _actions["InOutPoints/ResetInPoint"],
                SIGNAL(triggered(bool)),
                _currentTimelinePlayer,
                SLOT(resetInPoint()));
            connect(
                _actions["InOutPoints/SetOutPoint"],
                SIGNAL(triggered(bool)),
                _currentTimelinePlayer,
                SLOT(setOutPoint()));
            connect(
                _actions["InOutPoints/ResetOutPoint"],
                SIGNAL(triggered(bool)),
                _currentTimelinePlayer,
                SLOT(resetOutPoint()));
        }
        _layersUpdate();
        _timelineUpdate();
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

    void MainWindow::_layersUpdate()
    {
        for (const auto& i : _actionToLayer.keys())
        {
            _layersActionGroup->removeAction(i);
            i->setParent(nullptr);
            delete i;
        }
        _actionToLayer.clear();
        _layersMenu->clear();
        if (_currentTimelinePlayer)
        {
            const auto& videoInfo = _currentTimelinePlayer->videoInfo();
            const int videoLayer = _currentTimelinePlayer->videoLayer();
            for (size_t i = 0; i < videoInfo.size(); ++i)
            {
                auto action = new QAction;
                action->setCheckable(true);
                action->setChecked(i == videoLayer);
                action->setText(QString::fromUtf8(videoInfo[i].name.c_str()));
                _layersActionGroup->addAction(action);
                _actionToLayer[action] = i;
                _layersMenu->addAction(action);
            }
        }
    }

    void MainWindow::_playbackUpdate()
    {
        timeline::Playback playback = timeline::Playback::Stop;
        if (_currentTimelinePlayer)
        {
            playback = _currentTimelinePlayer->playback();
        }
        _actions["Playback/Stop"]->setChecked(timeline::Playback::Stop == playback);
        _actions["Playback/Forward"]->setChecked(timeline::Playback::Forward == playback);
        _actions["Playback/Reverse"]->setChecked(timeline::Playback::Reverse == playback);
    }

    void MainWindow::_timelineUpdate()
    {
        _actions["File/Close"]->setEnabled(!_timelinePlayers.empty());
        _actions["File/CloseAll"]->setEnabled(!_timelinePlayers.empty());
        _actions["File/Next"]->setEnabled(_timelinePlayers.size() > 1);
        _actions["File/Prev"]->setEnabled(_timelinePlayers.size() > 1);

        if (_currentTimelinePlayer)
        {
            _actions["Playback/Stop"]->setEnabled(true);
            _actions["Playback/Forward"]->setEnabled(true);
            _actions["Playback/Reverse"]->setEnabled(true);
            const auto playbackAction = _playbackToActions.find(_currentTimelinePlayer->playback());
            if (playbackAction != _playbackToActions.end())
            {
                playbackAction.value()->setChecked(true);
            }
            _actions["Playback/Toggle"]->setEnabled(true);

            _actions["Playback/Loop"]->setEnabled(true);
            _actions["Playback/Once"]->setEnabled(true);
            _actions["Playback/PingPong"]->setEnabled(true);
            const auto loopAction = _loopToActions.find(_currentTimelinePlayer->loop());
            if (loopAction != _loopToActions.end())
            {
                loopAction.value()->setChecked(true);
            }

            _actions["Time/Start"]->setEnabled(true);
            _actions["Time/End"]->setEnabled(true);
            _actions["Time/FramePrev"]->setEnabled(true);
            _actions["Time/FramePrevX10"]->setEnabled(true);
            _actions["Time/FramePrevX100"]->setEnabled(true);
            _actions["Time/FrameNext"]->setEnabled(true);
            _actions["Time/FrameNextX10"]->setEnabled(true);
            _actions["Time/FrameNextX100"]->setEnabled(true);

            _actions["InOutPoints/SetInPoint"]->setEnabled(true);
            _actions["InOutPoints/ResetInPoint"]->setEnabled(true);
            _actions["InOutPoints/SetOutPoint"]->setEnabled(true);
            _actions["InOutPoints/ResetOutPoint"]->setEnabled(true);
        }
        else
        {
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

            _actions["Time/Start"]->setEnabled(false);
            _actions["Time/End"]->setEnabled(false);
            _actions["Time/FramePrev"]->setEnabled(false);
            _actions["Time/FramePrevX10"]->setEnabled(false);
            _actions["Time/FramePrevX100"]->setEnabled(false);
            _actions["Time/FrameNext"]->setEnabled(false);
            _actions["Time/FrameNextX10"]->setEnabled(false);
            _actions["Time/FrameNextX100"]->setEnabled(false);

            _actions["InOutPoints/SetInPoint"]->setEnabled(false);
            _actions["InOutPoints/ResetInPoint"]->setEnabled(false);
            _actions["InOutPoints/SetOutPoint"]->setEnabled(false);
            _actions["InOutPoints/ResetOutPoint"]->setEnabled(false);
        }

        _tabWidget->setCurrentIndex(_timelinePlayers.indexOf(_currentTimelinePlayer));

        if (_secondaryWindow)
        {
            _secondaryWindow->setTimelinePlayer(_currentTimelinePlayer);
        }
    }
}
