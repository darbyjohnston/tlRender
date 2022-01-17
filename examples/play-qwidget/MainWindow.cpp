// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2022 Darby Johnston
// All rights reserved.

#include "MainWindow.h"

#include "App.h"
#include "CompareTool.h"
#include "FilesTool.h"
#include "ImageTool.h"
#include "SettingsTool.h"

#include <tlrCore/File.h>
#include <tlrCore/String.h>
#include <tlrCore/StringFormat.h>

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
        FilesModel* filesModel,
        LayersModel* layersModel,
        SettingsObject* settingsObject,
        qt::TimeObject* timeObject,
        const std::shared_ptr<core::Context>& context,
        QWidget* parent) :
        QMainWindow(parent),
        _filesModel(filesModel),
        _layersModel(layersModel),
        _settingsObject(settingsObject),
        _timeObject(timeObject)
    {
        setFocusPolicy(Qt::ClickFocus);
        setAcceptDrops(true);

        _context = context;

        _actions["File/Open"] = new QAction(this);
        _actions["File/Open"]->setText(tr("Open"));
        _actions["File/Open"]->setShortcut(QKeySequence::Open);
        _actions["File/OpenWithAudio"] = new QAction(this);
        _actions["File/OpenWithAudio"]->setText(tr("Open with Audio"));
        _actions["File/OpenWithAudio"]->setShortcut(QKeySequence(Qt::CTRL + Qt::SHIFT + Qt::Key_O));
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

        _actions["Playback/Start"] = new QAction(this);
        _actions["Playback/Start"]->setText(tr("Go To Start"));
        _actions["Playback/Start"]->setIcon(QIcon(":/Icons/TimeStart.svg"));
        _actions["Playback/Start"]->setShortcut(QKeySequence(Qt::Key_Home));
        _actions["Playback/End"] = new QAction(this);
        _actions["Playback/End"]->setText(tr("Go To End"));
        _actions["Playback/End"]->setIcon(QIcon(":/Icons/TimeEnd.svg"));
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

        _actions["Playback/SetInPoint"] = new QAction(this);
        _actions["Playback/SetInPoint"]->setText(tr("Set In Point"));
        _actions["Playback/SetInPoint"]->setIcon(QIcon(":/Icons/TimeStart.svg"));
        _actions["Playback/SetInPoint"]->setShortcut(QKeySequence(Qt::Key_I));
        _actions["Playback/ResetInPoint"] = new QAction(this);
        _actions["Playback/ResetInPoint"]->setText(tr("Reset In Point"));
        _actions["Playback/ResetInPoint"]->setIcon(QIcon(":/Icons/Reset.svg"));
        _actions["Playback/ResetInPoint"]->setShortcut(QKeySequence(Qt::SHIFT + Qt::Key_I));
        _actions["Playback/SetOutPoint"] = new QAction(this);
        _actions["Playback/SetOutPoint"]->setText(tr("Set Out Point"));
        _actions["Playback/SetOutPoint"]->setIcon(QIcon(":/Icons/TimeEnd.svg"));
        _actions["Playback/SetOutPoint"]->setShortcut(QKeySequence(Qt::Key_O));
        _actions["Playback/ResetOutPoint"] = new QAction(this);
        _actions["Playback/ResetOutPoint"]->setText(tr("Reset Out Point"));
        _actions["Playback/ResetOutPoint"]->setIcon(QIcon(":/Icons/Reset.svg"));
        _actions["Playback/ResetOutPoint"]->setShortcut(QKeySequence(Qt::SHIFT + Qt::Key_O));

        auto fileMenu = new QMenu;
        fileMenu->setTitle(tr("&File"));
        fileMenu->addAction(_actions["File/Open"]);
        fileMenu->addAction(_actions["File/OpenWithAudio"]);
        fileMenu->addAction(_actions["File/Close"]);
        fileMenu->addAction(_actions["File/CloseAll"]);
        _recentFilesMenu = new QMenu;
        _recentFilesMenu->setTitle(tr("&Recent Files"));
        fileMenu->addMenu(_recentFilesMenu);
        fileMenu->addSeparator();
        fileMenu->addAction(_actions["File/Next"]);
        fileMenu->addAction(_actions["File/Prev"]);
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
        playbackMenu->addSeparator();
        playbackMenu->addAction(_actions["Playback/Start"]);
        playbackMenu->addAction(_actions["Playback/End"]);
        playbackMenu->addSeparator();
        playbackMenu->addAction(_actions["Playback/FramePrev"]);
        playbackMenu->addAction(_actions["Playback/FramePrevX10"]);
        playbackMenu->addAction(_actions["Playback/FramePrevX100"]);
        playbackMenu->addAction(_actions["Playback/FrameNext"]);
        playbackMenu->addAction(_actions["Playback/FrameNextX10"]);
        playbackMenu->addAction(_actions["Playback/FrameNextX100"]);
        playbackMenu->addSeparator();
        playbackMenu->addAction(_actions["Playback/SetInPoint"]);
        playbackMenu->addAction(_actions["Playback/ResetInPoint"]);
        playbackMenu->addAction(_actions["Playback/SetOutPoint"]);
        playbackMenu->addAction(_actions["Playback/ResetOutPoint"]);

        auto toolsMenu = new QMenu;
        toolsMenu->setTitle(tr("&Tools"));

        auto menuBar = new QMenuBar;
        menuBar->addMenu(fileMenu);
        menuBar->addMenu(windowMenu);
        menuBar->addMenu(playbackMenu);
        menuBar->addMenu(toolsMenu);
        setMenuBar(menuBar);

        _timelineWidget = new qwidget::TimelineWidget(context);
        _timelineWidget->setTimeObject(_timeObject);
        setCentralWidget(_timelineWidget);

        auto filesTool = new FilesTool(filesModel);
        auto filesToolDockWidget = new QDockWidget;
        filesToolDockWidget->setObjectName("Files");
        filesToolDockWidget->setWindowTitle(tr("Files"));
        filesToolDockWidget->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);
        filesToolDockWidget->setWidget(filesTool);
        filesToolDockWidget->hide();
        toolsMenu->addAction(filesToolDockWidget->toggleViewAction());
        addDockWidget(Qt::RightDockWidgetArea, filesToolDockWidget);

        auto layersTool = new LayersTool(layersModel);
        auto layersToolDockWidget = new QDockWidget;
        layersToolDockWidget->setObjectName("Layers");
        layersToolDockWidget->setWindowTitle(tr("Layers"));
        layersToolDockWidget->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);
        layersToolDockWidget->setWidget(layersTool);
        layersToolDockWidget->hide();
        toolsMenu->addAction(layersToolDockWidget->toggleViewAction());
        addDockWidget(Qt::RightDockWidgetArea, layersToolDockWidget);

        auto compareTool = new CompareTool();
        auto compareToolDockWidget = new QDockWidget;
        compareToolDockWidget->setObjectName("Compare");
        compareToolDockWidget->setWindowTitle(tr("Compare"));
        compareToolDockWidget->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);
        compareToolDockWidget->setWidget(compareTool);
        compareToolDockWidget->hide();
        toolsMenu->addAction(compareToolDockWidget->toggleViewAction());
        addDockWidget(Qt::RightDockWidgetArea, compareToolDockWidget);

        auto imageTool = new ImageTool();
        auto imageToolDockWidget = new QDockWidget;
        imageToolDockWidget->setObjectName("Image");
        imageToolDockWidget->setWindowTitle(tr("Image"));
        imageToolDockWidget->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);
        imageToolDockWidget->setWidget(imageTool);
        imageToolDockWidget->hide();
        toolsMenu->addAction(imageToolDockWidget->toggleViewAction());
        addDockWidget(Qt::RightDockWidgetArea, imageToolDockWidget);

        _audioTool = new AudioTool();
        auto audioSyncDockWidget = new QDockWidget;
        audioSyncDockWidget->setObjectName("Audio");
        audioSyncDockWidget->setWindowTitle(tr("Audio"));
        audioSyncDockWidget->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);
        audioSyncDockWidget->setWidget(_audioTool);
        audioSyncDockWidget->hide();
        toolsMenu->addAction(audioSyncDockWidget->toggleViewAction());
        addDockWidget(Qt::RightDockWidgetArea, audioSyncDockWidget);

        auto settingsTool = new SettingsTool(settingsObject, _timeObject);
        auto settingsDockWidget = new QDockWidget;
        settingsDockWidget->setObjectName("Settings");
        settingsDockWidget->setWindowTitle(tr("Settings"));
        settingsDockWidget->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);
        settingsDockWidget->setWidget(settingsTool);
        settingsDockWidget->hide();
        toolsMenu->addAction(settingsDockWidget->toggleViewAction());
        addDockWidget(Qt::RightDockWidgetArea, settingsDockWidget);

        _recentFilesUpdate();
        _filesCountUpdate();
        _playbackUpdate();
        _timelineUpdate();

        connect(
            _actions["File/Open"],
            SIGNAL(triggered()),
            qApp,
            SLOT(open()));
        connect(
            _actions["File/OpenWithAudio"],
            SIGNAL(triggered()),
            qApp,
            SLOT(openWithAudio()));
        connect(
            _actions["File/Close"],
            SIGNAL(triggered()),
            qApp,
            SLOT(close()));
        connect(
            _actions["File/CloseAll"],
            SIGNAL(triggered()),
            qApp,
            SLOT(closeAll()));
        connect(
            _recentFilesActionGroup,
            SIGNAL(triggered(QAction*)),
            SLOT(_recentFilesCallback(QAction*)));
        connect(
            _actions["File/Next"],
            SIGNAL(triggered()),
            _filesModel,
            SLOT(next()));
        connect(
            _actions["File/Prev"],
            SIGNAL(triggered()),
            _filesModel,
            SLOT(prev()));
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
            _playbackActionGroup,
            SIGNAL(triggered(QAction*)),
            SLOT(_playbackCallback(QAction*)));

        connect(
            _loopActionGroup,
            SIGNAL(triggered(QAction*)),
            SLOT(_loopCallback(QAction*)));

        connect(
            imageTool,
            SIGNAL(imageOptionsChanged(const tlr::render::ImageOptions&)),
            SLOT(_imageOptionsCallback(const tlr::render::ImageOptions&)));

        connect(
            _audioTool,
            SIGNAL(audioOffsetChanged(double)),
            SLOT(_audioOffsetCallback(double)));

        connect(
            filesModel,
            SIGNAL(countChanged(int)),
            SLOT(_filesCountCallback()));

        connect(
            _settingsObject,
            SIGNAL(recentFilesChanged(const QList<QString>&)),
            SLOT(_recentFilesCallback()));

        if (auto app = qobject_cast<App*>(qApp))
        {
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

    void MainWindow::setTimelinePlayer(qt::TimelinePlayer* timelinePlayer)
    {
        if (_timelinePlayer)
        {
            disconnect(
                _timelinePlayer,
                SIGNAL(playbackChanged(tlr::timeline::Playback)),
                this,
                SLOT(_playbackCallback(tlr::timeline::Playback)));
            disconnect(
                _timelinePlayer,
                SIGNAL(loopChanged(tlr::timeline::Loop)),
                this,
                SLOT(_loopCallback(tlr::timeline::Loop)));
            disconnect(
                _timelinePlayer,
                SIGNAL(audioOffsetChanged(double)),
                _audioTool,
                SLOT(setAudioOffset(double)));

            disconnect(
                _actions["Playback/SetInPoint"],
                SIGNAL(triggered(bool)),
                _timelinePlayer,
                SLOT(setInPoint()));
            disconnect(
                _actions["Playback/ResetInPoint"],
                SIGNAL(triggered(bool)),
                _timelinePlayer,
                SLOT(resetInPoint()));
            disconnect(
                _actions["Playback/SetOutPoint"],
                SIGNAL(triggered(bool)),
                _timelinePlayer,
                SLOT(setOutPoint()));
            disconnect(
                _actions["Playback/ResetOutPoint"],
                SIGNAL(triggered(bool)),
                _timelinePlayer,
                SLOT(resetOutPoint()));
        }

        _timelinePlayer = timelinePlayer;

        if (_timelinePlayer)
        {
            connect(
                _timelinePlayer,
                SIGNAL(playbackChanged(tlr::timeline::Playback)),
                SLOT(_playbackCallback(tlr::timeline::Playback)));
            connect(
                _timelinePlayer,
                SIGNAL(loopChanged(tlr::timeline::Loop)),
                SLOT(_loopCallback(tlr::timeline::Loop)));
            connect(
                _timelinePlayer,
                SIGNAL(audioOffsetChanged(double)),
                _audioTool,
                SLOT(setAudioOffset(double)));

            connect(
                _actions["Playback/SetInPoint"],
                SIGNAL(triggered(bool)),
                _timelinePlayer,
                SLOT(setInPoint()));
            connect(
                _actions["Playback/ResetInPoint"],
                SIGNAL(triggered(bool)),
                _timelinePlayer,
                SLOT(resetInPoint()));
            connect(
                _actions["Playback/SetOutPoint"],
                SIGNAL(triggered(bool)),
                _timelinePlayer,
                SLOT(setOutPoint()));
            connect(
                _actions["Playback/ResetOutPoint"],
                SIGNAL(triggered(bool)),
                _timelinePlayer,
                SLOT(resetOutPoint()));
        }

        _playbackUpdate();
        _timelineUpdate();
    }

    void MainWindow::setColorConfig(const imaging::ColorConfig& colorConfig)
    {
        if (colorConfig != _colorConfig)
            return;
        _colorConfig = colorConfig;
        _timelineWidget->setColorConfig(_colorConfig);
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

    void MainWindow::_filesCountCallback()
    {
        _filesCountUpdate();
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
                _secondaryWindow->setTimelinePlayer(_timelinePlayer);

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

    void MainWindow::_playbackCallback(QAction* action)
    {
        if (_timelinePlayer)
        {
            const auto i = _actionToPlayback.find(action);
            if (i != _actionToPlayback.end())
            {
                _timelinePlayer->setPlayback(i.value());
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
        if (_timelinePlayer)
        {
            const auto i = _actionToLoop.find(action);
            if (i != _actionToLoop.end())
            {
                _timelinePlayer->setLoop(i.value());
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
        if (_timelinePlayer)
        {
            _timelinePlayer->stop();
        }
    }

    void MainWindow::_forwardCallback()
    {
        if (_timelinePlayer)
        {
            _timelinePlayer->forward();
        }
    }

    void MainWindow::_reverseCallback()
    {
        if (_timelinePlayer)
        {
            _timelinePlayer->reverse();
        }
    }

    void MainWindow::_togglePlaybackCallback()
    {
        if (_timelinePlayer)
        {
            _timelinePlayer->togglePlayback();
        }
    }

    void MainWindow::_startCallback()
    {
        if (_timelinePlayer)
        {
            _timelinePlayer->start();
        }
    }

    void MainWindow::_endCallback()
    {
        if (_timelinePlayer)
        {
            _timelinePlayer->end();
        }
    }

    void MainWindow::_framePrevCallback()
    {
        if (_timelinePlayer)
        {
            _timelinePlayer->framePrev();
        }
    }

    void MainWindow::_framePrevX10Callback()
    {
        if (_timelinePlayer)
        {
            _timelinePlayer->timeAction(timeline::TimeAction::FramePrevX10);
        }
    }

    void MainWindow::_framePrevX100Callback()
    {
        if (_timelinePlayer)
        {
            _timelinePlayer->timeAction(timeline::TimeAction::FramePrevX100);
        }
    }

    void MainWindow::_frameNextCallback()
    {
        if (_timelinePlayer)
        {
            _timelinePlayer->frameNext();
        }
    }

    void MainWindow::_frameNextX10Callback()
    {
        if (_timelinePlayer)
        {
            _timelinePlayer->timeAction(timeline::TimeAction::FrameNextX10);
        }
    }

    void MainWindow::_frameNextX100Callback()
    {
        if (_timelinePlayer)
        {
            _timelinePlayer->timeAction(timeline::TimeAction::FrameNextX100);
        }
    }

    void MainWindow::_imageOptionsCallback(const render::ImageOptions& value)
    {
        _timelineWidget->setImageOptions(value);
    }

    void MainWindow::_audioOffsetCallback(double value)
    {
        if (_timelinePlayer)
        {
            _timelinePlayer->setAudioOffset(value);
        }
    }

    void MainWindow::_saveSettingsCallback()
    {
        QSettings settings;
        settings.setValue("geometry", saveGeometry());
        settings.setValue("windowState", saveState());
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

    void MainWindow::_filesCountUpdate()
    {
        const int count = _filesModel->rowCount();
        _actions["File/Close"]->setEnabled(count > 0);
        _actions["File/CloseAll"]->setEnabled(count > 0);
        _actions["File/Next"]->setEnabled(count > 1);
        _actions["File/Prev"]->setEnabled(count > 1);
    }

    void MainWindow::_playbackUpdate()
    {
        timeline::Playback playback = timeline::Playback::Stop;
        if (_timelinePlayer)
        {
            playback = _timelinePlayer->playback();
        }
        _actions["Playback/Stop"]->setChecked(timeline::Playback::Stop == playback);
        _actions["Playback/Forward"]->setChecked(timeline::Playback::Forward == playback);
        _actions["Playback/Reverse"]->setChecked(timeline::Playback::Reverse == playback);
    }

    void MainWindow::_timelineUpdate()
    {
        if (_timelinePlayer)
        {
            _actions["Playback/Stop"]->setEnabled(true);
            _actions["Playback/Forward"]->setEnabled(true);
            _actions["Playback/Reverse"]->setEnabled(true);
            const auto playbackAction = _playbackToActions.find(_timelinePlayer->playback());
            if (playbackAction != _playbackToActions.end())
            {
                playbackAction.value()->setChecked(true);
            }
            _actions["Playback/Toggle"]->setEnabled(true);

            _actions["Playback/Loop"]->setEnabled(true);
            _actions["Playback/Once"]->setEnabled(true);
            _actions["Playback/PingPong"]->setEnabled(true);
            const auto loopAction = _loopToActions.find(_timelinePlayer->loop());
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

        _timelineWidget->setTimelinePlayer(_timelinePlayer);

        _audioTool->setAudioOffset(_timelinePlayer ? _timelinePlayer->audioOffset() : 0.0);

        if (_secondaryWindow)
        {
            _secondaryWindow->setTimelinePlayer(_timelinePlayer);
        }
    }
}
