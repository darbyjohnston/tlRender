// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2022 Darby Johnston
// All rights reserved.

#include <tlPlay/MainWindow.h>

#include <tlPlay/App.h>

#include <tlQWidget/Util.h>

#include <tlCore/File.h>
#include <tlCore/String.h>
#include <tlCore/StringFormat.h>

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

namespace tl
{
    namespace play
    {
        namespace
        {
            const size_t errorTimeout = 5000;
        }

        MainWindow::MainWindow(App* app, QWidget* parent) :
            QMainWindow(parent),
            _app(app)
        {
            setFocusPolicy(Qt::ClickFocus);
            setAcceptDrops(true);

            _actions["File/Open"] = new QAction(this);
            _actions["File/Open"]->setText(tr("Open"));
            _actions["File/Open"]->setShortcut(QKeySequence::Open);
            _actions["File/OpenWithAudio"] = new QAction(this);
            _actions["File/OpenWithAudio"]->setText(tr("Open With Audio"));
            _actions["File/OpenWithAudio"]->setShortcut(QKeySequence(Qt::CTRL | Qt::SHIFT | Qt::Key_O));
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
            _actions["File/NextLayer"] = new QAction(this);
            _actions["File/NextLayer"]->setText(tr("Next Layer"));
            _actions["File/NextLayer"]->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_Equal));
            _actions["File/PrevLayer"] = new QAction(this);
            _actions["File/PrevLayer"]->setText(tr("Previous Layer"));
            _actions["File/PrevLayer"]->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_Minus));
            _recentFilesActionGroup = new QActionGroup(this);
            _actions["File/Exit"] = new QAction(this);
            _actions["File/Exit"]->setText(tr("Exit"));
            _actions["File/Exit"]->setShortcut(QKeySequence::Quit);

            _actions["Window/Resize1280x720"] = new QAction(this);
            _actions["Window/Resize1280x720"]->setText(tr("Resize 1280x720"));
            _actions["Window/Resize1920x1080"] = new QAction(this);
            _actions["Window/Resize1920x1080"]->setText(tr("Resize 1920x1080"));
            _actions["Window/Resize1920x1080"] = new QAction(this);
            _actions["Window/Resize1920x1080"]->setText(tr("Resize 1920x1080"));
            _actions["Window/FullScreen"] = new QAction(this);
            _actions["Window/FullScreen"]->setText(tr("Full Screen"));
            _actions["Window/FullScreen"]->setShortcut(QKeySequence(Qt::Key_U));
            _actions["Window/FloatOnTop"] = new QAction(this);
            _actions["Window/FloatOnTop"]->setCheckable(true);
            _actions["Window/FloatOnTop"]->setText(tr("Float On Top"));
            _actions["Window/Secondary"] = new QAction(this);
            _actions["Window/Secondary"]->setCheckable(true);
            _actions["Window/Secondary"]->setText(tr("Secondary"));
            _actions["Window/Secondary"]->setShortcut(QKeySequence(Qt::Key_Y));
            _actions["Window/SecondaryFloatOnTop"] = new QAction(this);
            _actions["Window/SecondaryFloatOnTop"]->setCheckable(true);
            _actions["Window/SecondaryFloatOnTop"]->setText(tr("Secondary Float On Top"));

            _actions["Image/RedChannel"] = new QAction(this);
            _actions["Image/RedChannel"]->setCheckable(true);
            _actions["Image/RedChannel"]->setText(tr("Red Channel"));
            _actions["Image/RedChannel"]->setShortcut(QKeySequence(Qt::Key_R));
            _actions["Image/GreenChannel"] = new QAction(this);
            _actions["Image/GreenChannel"]->setCheckable(true);
            _actions["Image/GreenChannel"]->setText(tr("Green Channel"));
            _actions["Image/GreenChannel"]->setShortcut(QKeySequence(Qt::Key_G));
            _actions["Image/BlueChannel"] = new QAction(this);
            _actions["Image/BlueChannel"]->setCheckable(true);
            _actions["Image/BlueChannel"]->setText(tr("Blue Channel"));
            _actions["Image/BlueChannel"]->setShortcut(QKeySequence(Qt::Key_B));
            _actions["Image/AlphaChannel"] = new QAction(this);
            _actions["Image/AlphaChannel"]->setCheckable(true);
            _actions["Image/AlphaChannel"]->setText(tr("Alpha Channel"));
            _actions["Image/AlphaChannel"]->setShortcut(QKeySequence(Qt::Key_A));
            _channelsActionGroup = new QActionGroup(this);
            _channelsActionGroup->setExclusionPolicy(QActionGroup::ExclusionPolicy::ExclusiveOptional);
            _channelsActionGroup->addAction(_actions["Image/RedChannel"]);
            _channelsActionGroup->addAction(_actions["Image/GreenChannel"]);
            _channelsActionGroup->addAction(_actions["Image/BlueChannel"]);
            _channelsActionGroup->addAction(_actions["Image/AlphaChannel"]);
            _actionToChannels[_actions["Image/RedChannel"]] = render::Channels::Red;
            _actionToChannels[_actions["Image/GreenChannel"]] = render::Channels::Green;
            _actionToChannels[_actions["Image/BlueChannel"]] = render::Channels::Blue;
            _actionToChannels[_actions["Image/AlphaChannel"]] = render::Channels::Alpha;
            _channelsToActions[render::Channels::Red] = _actions["Image/RedChannel"];
            _channelsToActions[render::Channels::Green] = _actions["Image/GreenChannel"];
            _channelsToActions[render::Channels::Blue] = _actions["Image/BlueChannel"];
            _channelsToActions[render::Channels::Alpha] = _actions["Image/AlphaChannel"];

            _actions["Image/MirrorX"] = new QAction(this);
            _actions["Image/MirrorX"]->setText(tr("Mirror Horizontal"));
            _actions["Image/MirrorX"]->setShortcut(QKeySequence(Qt::Key_H));
            _actions["Image/MirrorX"]->setCheckable(true);
            _actions["Image/MirrorY"] = new QAction(this);
            _actions["Image/MirrorY"]->setText(tr("Mirror Vertical"));
            _actions["Image/MirrorY"]->setShortcut(QKeySequence(Qt::Key_V));
            _actions["Image/MirrorY"]->setCheckable(true);

            _actions["Playback/Stop"] = new QAction(this);
            _actions["Playback/Stop"]->setCheckable(true);
            _actions["Playback/Stop"]->setText(tr("Stop Playback"));
            _actions["Playback/Stop"]->setIcon(QIcon(":/Icons/PlaybackStop.svg"));
            _actions["Playback/Stop"]->setShortcut(QKeySequence(Qt::Key_K));
            _actions["Playback/Forward"] = new QAction(this);
            _actions["Playback/Forward"]->setCheckable(true);
            _actions["Playback/Forward"]->setText(tr("Forward Playback"));
            _actions["Playback/Forward"]->setIcon(QIcon(":/Icons/PlaybackForward.svg"));
            _actions["Playback/Forward"]->setShortcut(QKeySequence(Qt::Key_L));
            _actions["Playback/Reverse"] = new QAction(this);
            _actions["Playback/Reverse"]->setCheckable(true);
            _actions["Playback/Reverse"]->setText(tr("Reverse Playback"));
            _actions["Playback/Reverse"]->setIcon(QIcon(":/Icons/PlaybackReverse.svg"));
            _actions["Playback/Reverse"]->setShortcut(QKeySequence(Qt::Key_J));
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
            _actions["Playback/SetInPoint"]->setShortcut(QKeySequence(Qt::Key_I));
            _actions["Playback/ResetInPoint"] = new QAction(this);
            _actions["Playback/ResetInPoint"]->setText(tr("Reset In Point"));
            _actions["Playback/ResetInPoint"]->setShortcut(QKeySequence(Qt::SHIFT + Qt::Key_I));
            _actions["Playback/SetOutPoint"] = new QAction(this);
            _actions["Playback/SetOutPoint"]->setText(tr("Set Out Point"));
            _actions["Playback/SetOutPoint"]->setShortcut(QKeySequence(Qt::Key_O));
            _actions["Playback/ResetOutPoint"] = new QAction(this);
            _actions["Playback/ResetOutPoint"]->setText(tr("Reset Out Point"));
            _actions["Playback/ResetOutPoint"]->setShortcut(QKeySequence(Qt::SHIFT + Qt::Key_O));

            _actions["Playback/FocusCurrentFrame"] = new QAction(this);
            _actions["Playback/FocusCurrentFrame"]->setText(tr("Focus Current Frame"));
            _actions["Playback/FocusCurrentFrame"]->setShortcut(QKeySequence(Qt::Key_F));

            _actions["Audio/IncreaseVolume"] = new QAction(this);
            _actions["Audio/IncreaseVolume"]->setText(tr("Increase Volume"));
            _actions["Audio/IncreaseVolume"]->setShortcut(QKeySequence(Qt::Key_Period));
            _actions["Audio/DecreaseVolume"] = new QAction(this);
            _actions["Audio/DecreaseVolume"]->setText(tr("Decrease Volume"));
            _actions["Audio/DecreaseVolume"]->setShortcut(QKeySequence(Qt::Key_Comma));
            _actions["Audio/Mute"] = new QAction(this);
            _actions["Audio/Mute"]->setCheckable(true);
            _actions["Audio/Mute"]->setText(tr("Mute"));
            _actions["Audio/Mute"]->setShortcut(QKeySequence(Qt::Key_M));

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
            fileMenu->addAction(_actions["File/NextLayer"]);
            fileMenu->addAction(_actions["File/PrevLayer"]);
            fileMenu->addSeparator();
            fileMenu->addAction(_actions["File/Exit"]);

            auto windowMenu = new QMenu;
            windowMenu->setTitle(tr("&Window"));
            windowMenu->addAction(_actions["Window/Resize1280x720"]);
            windowMenu->addAction(_actions["Window/Resize1920x1080"]);
            windowMenu->addSeparator();
            windowMenu->addAction(_actions["Window/FullScreen"]);
            windowMenu->addAction(_actions["Window/FloatOnTop"]);
            windowMenu->addSeparator();
            windowMenu->addAction(_actions["Window/Secondary"]);
            windowMenu->addAction(_actions["Window/SecondaryFloatOnTop"]);

            auto imageMenu = new QMenu;
            imageMenu->setTitle(tr("&Image"));
            imageMenu->addAction(_actions["Image/RedChannel"]);
            imageMenu->addAction(_actions["Image/GreenChannel"]);
            imageMenu->addAction(_actions["Image/BlueChannel"]);
            imageMenu->addAction(_actions["Image/AlphaChannel"]);
            imageMenu->addSeparator();
            imageMenu->addAction(_actions["Image/MirrorX"]);
            imageMenu->addAction(_actions["Image/MirrorY"]);

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
            playbackMenu->addSeparator();
            playbackMenu->addAction(_actions["Playback/FocusCurrentFrame"]);

            auto audioMenu = new QMenu;
            audioMenu->setTitle(tr("&Audio"));
            audioMenu->addAction(_actions["Audio/IncreaseVolume"]);
            audioMenu->addAction(_actions["Audio/DecreaseVolume"]);
            audioMenu->addAction(_actions["Audio/Mute"]);

            auto toolsMenu = new QMenu;
            toolsMenu->setTitle(tr("&Tools"));

            auto menuBar = new QMenuBar;
            menuBar->addMenu(fileMenu);
            menuBar->addMenu(windowMenu);
            menuBar->addMenu(imageMenu);
            menuBar->addMenu(playbackMenu);
            menuBar->addMenu(audioMenu);
            menuBar->addMenu(toolsMenu);
            setMenuBar(menuBar);

            _timelineWidget = new qwidget::TimelineWidget(app->getContext());
            _timelineWidget->setTimeObject(app->timeObject());
            setCentralWidget(_timelineWidget);

            _filesTool = new FilesTool(app->filesModel(), app->getContext());
            auto fileDockWidget = new QDockWidget;
            fileDockWidget->setObjectName("Files");
            fileDockWidget->setWindowTitle(tr("Files"));
            fileDockWidget->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);
            fileDockWidget->setStyleSheet(qwidget::dockWidgetStyleSheet());
            fileDockWidget->setWidget(_filesTool);
            fileDockWidget->hide();
            fileDockWidget->toggleViewAction()->setShortcut(QKeySequence(Qt::Key_F1));
            toolsMenu->addAction(fileDockWidget->toggleViewAction());
            addDockWidget(Qt::RightDockWidgetArea, fileDockWidget);

            _compareTool = new CompareTool(app->filesModel(), app->getContext());
            auto compareDockWidget = new QDockWidget;
            compareDockWidget->setObjectName("Compare");
            compareDockWidget->setWindowTitle(tr("Compare"));
            compareDockWidget->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);
            compareDockWidget->setStyleSheet(qwidget::dockWidgetStyleSheet());
            compareDockWidget->setWidget(_compareTool);
            compareDockWidget->hide();
            compareDockWidget->toggleViewAction()->setShortcut(QKeySequence(Qt::Key_F2));
            toolsMenu->addAction(compareDockWidget->toggleViewAction());
            addDockWidget(Qt::RightDockWidgetArea, compareDockWidget);

            _colorTool = new ColorTool(app->colorModel());
            auto colorDockWidget = new QDockWidget;
            colorDockWidget->setObjectName("Color");
            colorDockWidget->setWindowTitle(tr("Color"));
            colorDockWidget->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);
            colorDockWidget->setStyleSheet(qwidget::dockWidgetStyleSheet());
            colorDockWidget->setWidget(_colorTool);
            colorDockWidget->hide();
            colorDockWidget->toggleViewAction()->setShortcut(QKeySequence(Qt::Key_F3));
            toolsMenu->addAction(colorDockWidget->toggleViewAction());
            addDockWidget(Qt::RightDockWidgetArea, colorDockWidget);

            _imageTool = new ImageTool();
            auto imageDockWidget = new QDockWidget;
            imageDockWidget->setObjectName("Image");
            imageDockWidget->setWindowTitle(tr("Image"));
            imageDockWidget->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);
            imageDockWidget->setStyleSheet(qwidget::dockWidgetStyleSheet());
            imageDockWidget->setWidget(_imageTool);
            imageDockWidget->hide();
            imageDockWidget->toggleViewAction()->setShortcut(QKeySequence(Qt::Key_F4));
            toolsMenu->addAction(imageDockWidget->toggleViewAction());
            addDockWidget(Qt::RightDockWidgetArea, imageDockWidget);

            _infoTool = new InfoTool();
            auto infoDockWidget = new QDockWidget;
            infoDockWidget->setObjectName("Info");
            infoDockWidget->setWindowTitle(tr("Information"));
            infoDockWidget->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);
            infoDockWidget->setStyleSheet(qwidget::dockWidgetStyleSheet());
            infoDockWidget->setWidget(_infoTool);
            infoDockWidget->hide();
            infoDockWidget->toggleViewAction()->setShortcut(QKeySequence(Qt::Key_F5));
            toolsMenu->addAction(infoDockWidget->toggleViewAction());
            addDockWidget(Qt::RightDockWidgetArea, infoDockWidget);

            _audioTool = new AudioTool();
            auto audioDockWidget = new QDockWidget;
            audioDockWidget->setObjectName("Audio");
            audioDockWidget->setWindowTitle(tr("Audio"));
            audioDockWidget->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);
            audioDockWidget->setStyleSheet(qwidget::dockWidgetStyleSheet());
            audioDockWidget->setWidget(_audioTool);
            audioDockWidget->hide();
            audioDockWidget->toggleViewAction()->setShortcut(QKeySequence(Qt::Key_F6));
            toolsMenu->addAction(audioDockWidget->toggleViewAction());
            addDockWidget(Qt::RightDockWidgetArea, audioDockWidget);

            _settingsTool = new SettingsTool(app->settingsObject(), app->timeObject());
            auto settingsDockWidget = new QDockWidget;
            settingsDockWidget->setObjectName("Settings");
            settingsDockWidget->setWindowTitle(tr("Settings"));
            settingsDockWidget->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);
            settingsDockWidget->setStyleSheet(qwidget::dockWidgetStyleSheet());
            settingsDockWidget->setWidget(_settingsTool);
            settingsDockWidget->hide();
            settingsDockWidget->toggleViewAction()->setShortcut(QKeySequence(Qt::Key_F9));
            toolsMenu->addAction(settingsDockWidget->toggleViewAction());
            addDockWidget(Qt::RightDockWidgetArea, settingsDockWidget);

            _messagesTool = new MessagesTool(app->getContext());
            auto messagesDockWidget = new QDockWidget;
            messagesDockWidget->setObjectName("Messages");
            messagesDockWidget->setWindowTitle(tr("Messages"));
            messagesDockWidget->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);
            messagesDockWidget->setStyleSheet(qwidget::dockWidgetStyleSheet());
            messagesDockWidget->setWidget(_messagesTool);
            messagesDockWidget->hide();
            messagesDockWidget->toggleViewAction()->setShortcut(QKeySequence(Qt::Key_F10));
            toolsMenu->addAction(messagesDockWidget->toggleViewAction());
            addDockWidget(Qt::RightDockWidgetArea, messagesDockWidget);

            _systemLogTool = new SystemLogTool(app->getContext());
            auto systemLogDockWidget = new QDockWidget;
            systemLogDockWidget->setObjectName("SystemLog");
            systemLogDockWidget->setWindowTitle(tr("System Log"));
            systemLogDockWidget->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);
            systemLogDockWidget->setStyleSheet(qwidget::dockWidgetStyleSheet());
            systemLogDockWidget->setWidget(_systemLogTool);
            systemLogDockWidget->hide();
            systemLogDockWidget->toggleViewAction()->setShortcut(QKeySequence(Qt::Key_F11));
            toolsMenu->addAction(systemLogDockWidget->toggleViewAction());
            addDockWidget(Qt::RightDockWidgetArea, systemLogDockWidget);

            _infoLabel = new QLabel;
    
            _statusBar = new QStatusBar;
            _statusBar->addPermanentWidget(_infoLabel);
            setStatusBar(_statusBar);

            _recentFilesUpdate();
            _widgetUpdate();

            _filesObserver = observer::ListObserver<std::shared_ptr<FilesModelItem> >::create(
                app->filesModel()->observeFiles(),
                [this](const std::vector<std::shared_ptr<FilesModelItem> >&)
                {
                    _widgetUpdate();
                });
            _imageOptionsObserver = observer::ListObserver<render::ImageOptions>::create(
                app->filesModel()->observeImageOptions(),
                [this](const std::vector<render::ImageOptions>& value)
                {
                    _imageOptionsCallback(value);
                });
            _compareOptionsObserver = observer::ValueObserver<render::CompareOptions>::create(
                app->filesModel()->observeCompareOptions(),
                [this](const render::CompareOptions& value)
                {
                    _compareOptionsCallback2(value);
                });

            _colorConfigObserver = observer::ValueObserver<imaging::ColorConfig>::create(
                app->colorModel()->observeConfig(),
                [this](const imaging::ColorConfig& value)
                {
                    _colorConfig = value;
                    _widgetUpdate();
                });

            _logObserver = observer::ValueObserver<core::LogItem>::create(
                app->getContext()->getLogSystem()->observeLog(),
                [this](const core::LogItem& value)
                {
                    switch (value.type)
                    {
                    case core::LogType::Error:
                        _statusBar->showMessage(
                            QString(tr("ERROR: %1")).
                            arg(QString::fromUtf8(value.message.c_str())),
                            errorTimeout);
                        break;
                    default: break;
                    }
                });

            connect(
                _actions["File/Open"],
                &QAction::triggered,
                app,
                &App::openDialog);
            connect(
                _actions["File/OpenWithAudio"],
                &QAction::triggered,
                app,
                &App::openWithAudioDialog);
            connect(
                _actions["File/Close"],
                &QAction::triggered,
                [app]
                {
                    app->filesModel()->close();
                });
            connect(
                _actions["File/CloseAll"],
                &QAction::triggered,
                [app]
                {
                    app->filesModel()->closeAll();
                });
            connect(
                _recentFilesActionGroup,
                SIGNAL(triggered(QAction*)),
                SLOT(_recentFilesCallback(QAction*)));
            connect(
                _actions["File/Next"],
                &QAction::triggered,
                [app]
                {
                    app->filesModel()->next();
                });
            connect(
                _actions["File/Prev"],
                &QAction::triggered,
                [app]
                {
                    app->filesModel()->prev();
                });
            connect(
                _actions["File/NextLayer"],
                &QAction::triggered,
                [app]
                {
                    app->filesModel()->nextLayer();
                });
            connect(
                _actions["File/PrevLayer"],
                &QAction::triggered,
                [app]
                {
                    app->filesModel()->prevLayer();
                });
            connect(
                _actions["File/Exit"],
                &QAction::triggered,
                app,
                &App::quit);

            connect(
                _actions["Window/Resize1280x720"],
                &QAction::triggered,
                [this]
                {
                    resize(1280, 720);
                });
            connect(
                _actions["Window/Resize1920x1080"],
                &QAction::triggered,
                [this]
                {
                    resize(1920, 1080);
                });
            connect(
                _actions["Window/FullScreen"],
                &QAction::triggered,
                [this]
                {
                    setWindowState(windowState() ^ Qt::WindowFullScreen);
                });
            connect(
                _actions["Window/FloatOnTop"],
                &QAction::toggled,
                [this](bool value)
                {
                    _floatOnTop = value;
                    if (_floatOnTop)
                    {
                        setWindowFlags(windowFlags() | Qt::WindowStaysOnTopHint);
                    }
                    else
                    {
                        setWindowFlags(windowFlags() & ~Qt::WindowStaysOnTopHint);
                    }
                    show();
                });
            connect(
                _actions["Window/Secondary"],
                SIGNAL(toggled(bool)),
                SLOT(_secondaryWindowCallback(bool)));
            connect(
                _actions["Window/SecondaryFloatOnTop"],
                &QAction::toggled,
                [this](bool value)
                {
                    _secondaryFloatOnTop = value;
                    if (_secondaryWindow)
                    {
                        if (_secondaryFloatOnTop)
                        {
                            _secondaryWindow->setWindowFlags(_secondaryWindow->windowFlags() | Qt::WindowStaysOnTopHint);
                        }
                        else
                        {
                            _secondaryWindow->setWindowFlags(_secondaryWindow->windowFlags() & ~Qt::WindowStaysOnTopHint);
                        }
                        _secondaryWindow->show();
                    }
                });

            connect(
                _channelsActionGroup,
                SIGNAL(triggered(QAction*)),
                SLOT(_channelsCallback(QAction*)));

            connect(
                _actions["Image/MirrorX"],
                &QAction::toggled,
                [this](bool value)
                {
                    if (!_imageOptions.empty())
                    {
                        render::ImageOptions imageOptions = _imageOptions[0];
                        imageOptions.mirror.x = value;
                        _app->filesModel()->setImageOptions(imageOptions);
                    }
                });
            connect(
                _actions["Image/MirrorY"],
                &QAction::toggled,
                [this](bool value)
                {
                    if (!_imageOptions.empty())
                    {
                        render::ImageOptions imageOptions = _imageOptions[0];
                        imageOptions.mirror.y = value;
                        _app->filesModel()->setImageOptions(imageOptions);
                    }
                });

            connect(
                _actions["Playback/Toggle"],
                &QAction::triggered,
                [this]
                {
                    if (!_timelinePlayers.empty())
                    {
                        _timelinePlayers[0]->togglePlayback();
                    }
                });
            connect(
                _actions["Playback/Start"],
                &QAction::triggered,
                [this]
                {
                    if (!_timelinePlayers.empty())
                    {
                        _timelinePlayers[0]->start();
                    }
                });
            connect(
                _actions["Playback/End"],
                &QAction::triggered,
                [this]
                {
                    if (!_timelinePlayers.empty())
                    {
                        _timelinePlayers[0]->end();
                    }
                });
            connect(
                _actions["Playback/FramePrev"],
                &QAction::triggered,
                [this]
                {
                    if (!_timelinePlayers.empty())
                    {
                        _timelinePlayers[0]->framePrev();
                    }
                });
            connect(
                _actions["Playback/FramePrevX10"],
                &QAction::triggered,
                [this]
                {
                    if (!_timelinePlayers.empty())
                    {
                        _timelinePlayers[0]->timeAction(timeline::TimeAction::FramePrevX10);
                    }
                });
            connect(
                _actions["Playback/FramePrevX100"],
                &QAction::triggered,
                [this]
                {
                    if (!_timelinePlayers.empty())
                    {
                        _timelinePlayers[0]->timeAction(timeline::TimeAction::FramePrevX100);
                    }
                });
            connect(
                _actions["Playback/FrameNext"],
                &QAction::triggered,
                [this]
                {
                    if (!_timelinePlayers.empty())
                    {
                        _timelinePlayers[0]->frameNext();
                    }
                });
            connect(
                _actions["Playback/FrameNextX10"],
                &QAction::triggered,
                [this]
                {
                    if (!_timelinePlayers.empty())
                    {
                        _timelinePlayers[0]->timeAction(timeline::TimeAction::FrameNextX10);
                    }
                });
            connect(
                _actions["Playback/FrameNextX100"],
                &QAction::triggered,
                [this]
                {
                    if (!_timelinePlayers.empty())
                    {
                        _timelinePlayers[0]->timeAction(timeline::TimeAction::FrameNextX100);
                    }
                });
            connect(
                _actions["Playback/FocusCurrentFrame"],
                &QAction::triggered,
                [this]
                {
                    _timelineWidget->focusCurrentFrame();
                });

            connect(
                _playbackActionGroup,
                SIGNAL(triggered(QAction*)),
                SLOT(_playbackCallback(QAction*)));

            connect(
                _loopActionGroup,
                SIGNAL(triggered(QAction*)),
                SLOT(_loopCallback(QAction*)));

            connect(
                _compareTool,
                SIGNAL(compareOptionsChanged(const tl::render::CompareOptions&)),
                SLOT(_compareOptionsCallback(const tl::render::CompareOptions&)));

            connect(
                _imageTool,
                SIGNAL(imageOptionsChanged(const tl::render::ImageOptions&)),
                SLOT(_imageOptionsCallback(const tl::render::ImageOptions&)));

            connect(
                _audioTool,
                &AudioTool::audioOffsetChanged,
                [this](double value)
                {
                    if (!_timelinePlayers.empty())
                    {
                        _timelinePlayers[0]->setAudioOffset(value);
                    }
                });

            connect(
                app->settingsObject(),
                SIGNAL(recentFilesChanged(const QList<QString>&)),
                SLOT(_recentFilesCallback()));

            QSettings settings;
            auto ba = settings.value(qt::versionedSettingsKey("MainWindow/geometry")).toByteArray();
            if (!ba.isEmpty())
            {
                restoreGeometry(settings.value(qt::versionedSettingsKey("MainWindow/geometry")).toByteArray());
            }
            else
            {
                resize(1280, 720);
            }
            ba = settings.value(qt::versionedSettingsKey("MainWindow/windowState")).toByteArray();
            if (!ba.isEmpty())
            {
                restoreState(settings.value(qt::versionedSettingsKey("MainWindow/windowState")).toByteArray());
            }
            if (settings.contains("MainWindow/FloatOnTop"))
            {
                _floatOnTop = settings.value("MainWindow/FloatOnTop").toBool();
                if (_floatOnTop)
                {
                    setWindowFlags(windowFlags() | Qt::WindowStaysOnTopHint);
                }
                else
                {
                    setWindowFlags(windowFlags() & ~Qt::WindowStaysOnTopHint);
                }
                QSignalBlocker blocker(_actions["Window/FloatOnTop"]);
                _actions["Window/FloatOnTop"]->setChecked(_floatOnTop);
            }
            if (settings.contains("MainWindow/SecondaryFloatOnTop"))
            {
                _secondaryFloatOnTop = settings.value("MainWindow/SecondaryFloatOnTop").toBool();
                QSignalBlocker blocker(_actions["Window/SecondaryFloatOnTop"]);
                _actions["Window/SecondaryFloatOnTop"]->setChecked(_secondaryFloatOnTop);
            }
        }

        MainWindow::~MainWindow()
        {
            QSettings settings;
            settings.setValue(qt::versionedSettingsKey("MainWindow/geometry"), saveGeometry());
            settings.setValue(qt::versionedSettingsKey("MainWindow/windowState"), saveState());
            settings.setValue("MainWindow/FloatOnTop", _floatOnTop);
            settings.setValue("MainWindow/SecondaryFloatOnTop", _secondaryFloatOnTop);
            if (_secondaryWindow)
            {
                delete _secondaryWindow;
                _secondaryWindow = nullptr;
            }
        }

        void MainWindow::setImageOptions(const std::vector<render::ImageOptions>& imageOptions)
        {
            if (imageOptions == _imageOptions)
                return;
            _imageOptions = imageOptions;
            _widgetUpdate();
        }

        void MainWindow::setTimelinePlayers(const std::vector<qt::TimelinePlayer*>& timelinePlayers)
        {
            if (!_timelinePlayers.empty())
            {
                disconnect(
                    _timelinePlayers[0],
                    SIGNAL(playbackChanged(tl::timeline::Playback)),
                    this,
                    SLOT(_playbackCallback(tl::timeline::Playback)));
                disconnect(
                    _timelinePlayers[0],
                    SIGNAL(loopChanged(tl::timeline::Loop)),
                    this,
                    SLOT(_loopCallback(tl::timeline::Loop)));
                disconnect(
                    _timelinePlayers[0],
                    SIGNAL(audioOffsetChanged(double)),
                    _audioTool,
                    SLOT(setAudioOffset(double)));

                disconnect(
                    _actions["Playback/SetInPoint"],
                    SIGNAL(triggered()),
                    _timelinePlayers[0],
                    SLOT(setInPoint()));
                disconnect(
                    _actions["Playback/ResetInPoint"],
                    SIGNAL(triggered()),
                    _timelinePlayers[0],
                    SLOT(resetInPoint()));
                disconnect(
                    _actions["Playback/SetOutPoint"],
                    SIGNAL(triggered()),
                    _timelinePlayers[0],
                    SLOT(setOutPoint()));
                disconnect(
                    _actions["Playback/ResetOutPoint"],
                    SIGNAL(triggered()),
                    _timelinePlayers[0],
                    SLOT(resetOutPoint()));

                disconnect(
                    _actions["Audio/IncreaseVolume"],
                    SIGNAL(triggered()),
                    _timelinePlayers[0],
                    SLOT(increaseVolume()));
                disconnect(
                    _actions["Audio/DecreaseVolume"],
                    SIGNAL(triggered()),
                    _timelinePlayers[0],
                    SLOT(decreaseVolume()));
                disconnect(
                    _actions["Audio/Mute"],
                    SIGNAL(toggled(bool)),
                    _timelinePlayers[0],
                    SLOT(setMute(bool)));
            }

            _timelinePlayers = timelinePlayers;

            if (!_timelinePlayers.empty())
            {
                connect(
                    _timelinePlayers[0],
                    SIGNAL(playbackChanged(tl::timeline::Playback)),
                    SLOT(_playbackCallback(tl::timeline::Playback)));
                connect(
                    _timelinePlayers[0],
                    SIGNAL(loopChanged(tl::timeline::Loop)),
                    SLOT(_loopCallback(tl::timeline::Loop)));
                connect(
                    _timelinePlayers[0],
                    SIGNAL(audioOffsetChanged(double)),
                    _audioTool,
                    SLOT(setAudioOffset(double)));

                connect(
                    _actions["Playback/SetInPoint"],
                    SIGNAL(triggered()),
                    _timelinePlayers[0],
                    SLOT(setInPoint()));
                connect(
                    _actions["Playback/ResetInPoint"],
                    SIGNAL(triggered()),
                    _timelinePlayers[0],
                    SLOT(resetInPoint()));
                connect(
                    _actions["Playback/SetOutPoint"],
                    SIGNAL(triggered()),
                    _timelinePlayers[0],
                    SLOT(setOutPoint()));
                connect(
                    _actions["Playback/ResetOutPoint"],
                    SIGNAL(triggered()),
                    _timelinePlayers[0],
                    SLOT(resetOutPoint()));

                connect(
                    _actions["Audio/IncreaseVolume"],
                    SIGNAL(triggered()),
                    _timelinePlayers[0],
                    SLOT(increaseVolume()));
                connect(
                    _actions["Audio/DecreaseVolume"],
                    SIGNAL(triggered()),
                    _timelinePlayers[0],
                    SLOT(decreaseVolume()));
                connect(
                    _actions["Audio/Mute"],
                    SIGNAL(toggled(bool)),
                    _timelinePlayers[0],
                    SLOT(setMute(bool)));
            }

            _widgetUpdate();
        }

        void MainWindow::closeEvent(QCloseEvent*)
        {
            if (_secondaryWindow)
            {
                delete _secondaryWindow;
                _secondaryWindow = nullptr;
            }
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
                    const QString fileName = urlList[i].toLocalFile();
                    _app->open(fileName);
                }
            }
        }

        void MainWindow::_recentFilesCallback(QAction* action)
        {
            const auto i = _actionToRecentFile.find(action);
            if (i != _actionToRecentFile.end())
            {
                _app->open(i.value());
            }
        }

        void MainWindow::_recentFilesCallback()
        {
            _recentFilesUpdate();
        }

        void MainWindow::_secondaryWindowCallback(bool value)
        {
            if (value && !_secondaryWindow)
            {
                _secondaryWindow = new SecondaryWindow(_app->getContext());
                _secondaryWindow->setColorConfig(_colorConfig);
                _secondaryWindow->setCompareOptions(_compareOptions);
                _secondaryWindow->setTimelinePlayers(_timelinePlayers);

                connect(
                    _secondaryWindow,
                    SIGNAL(destroyed(QObject*)),
                    SLOT(_secondaryWindowDestroyedCallback()));

                if (_secondaryFloatOnTop)
                {
                    _secondaryWindow->setWindowFlags(_secondaryWindow->windowFlags() | Qt::WindowStaysOnTopHint);
                }
                else
                {
                    _secondaryWindow->setWindowFlags(_secondaryWindow->windowFlags() & ~Qt::WindowStaysOnTopHint);
                }
                _secondaryWindow->show();
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

        void MainWindow::_channelsCallback(QAction* action)
        {
            if (!_imageOptions.empty())
            {
                const auto i = _actionToChannels.find(action);
                if (i != _actionToChannels.end())
                {
                    render::ImageOptions imageOptions = _imageOptions[0];
                    imageOptions.channels = action->isChecked() ? i.value() : render::Channels::Color;
                    _app->filesModel()->setImageOptions(imageOptions);
                }
            }
        }

        void MainWindow::_playbackCallback(QAction* action)
        {
            if (!_timelinePlayers.empty())
            {
                const auto i = _actionToPlayback.find(action);
                if (i != _actionToPlayback.end())
                {
                    _timelinePlayers[0]->setPlayback(i.value());
                }
            }
        }

        void MainWindow::_playbackCallback(tl::timeline::Playback value)
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
            if (!_timelinePlayers.empty())
            {
                const auto i = _actionToLoop.find(action);
                if (i != _actionToLoop.end())
                {
                    _timelinePlayers[0]->setLoop(i.value());
                }
            }
        }

        void MainWindow::_loopCallback(tl::timeline::Loop value)
        {
            const QSignalBlocker blocker(_loopActionGroup);
            const auto i = _loopToActions.find(value);
            if (i != _loopToActions.end())
            {
                i.value()->setChecked(true);
            }
        }

        void MainWindow::_imageOptionsCallback(const render::ImageOptions& value)
        {
            _app->filesModel()->setImageOptions(value);
        }

        void MainWindow::_imageOptionsCallback(const std::vector<render::ImageOptions>& value)
        {
            _imageOptions = value;
            _widgetUpdate();
        }

        void MainWindow::_compareOptionsCallback(const render::CompareOptions& value)
        {
            _app->filesModel()->setCompareOptions(value);
        }

        void MainWindow::_compareOptionsCallback2(const render::CompareOptions& value)
        {
            _compareOptions = value;
            _widgetUpdate();
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
            const auto& recentFiles = _app->settingsObject()->recentFiles();
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

        void MainWindow::_widgetUpdate()
        {
            const int count = _app->filesModel()->observeFiles()->getSize();
            _actions["File/Close"]->setEnabled(count > 0);
            _actions["File/CloseAll"]->setEnabled(count > 0);
            _actions["File/Next"]->setEnabled(count > 1);
            _actions["File/Prev"]->setEnabled(count > 1);
            _actions["File/NextLayer"]->setEnabled(count > 0);
            _actions["File/PrevLayer"]->setEnabled(count > 0);

            _actions["Image/RedChannel"]->setEnabled(count > 0);
            _actions["Image/GreenChannel"]->setEnabled(count > 0);
            _actions["Image/BlueChannel"]->setEnabled(count > 0);
            _actions["Image/AlphaChannel"]->setEnabled(count > 0);
            _actions["Image/MirrorX"]->setEnabled(count > 0);
            _actions["Image/MirrorY"]->setEnabled(count > 0);

            _actions["Playback/Stop"]->setEnabled(count > 0);
            _actions["Playback/Forward"]->setEnabled(count > 0);
            _actions["Playback/Reverse"]->setEnabled(count > 0);
            _actions["Playback/Toggle"]->setEnabled(count > 0);
            _actions["Playback/Loop"]->setEnabled(count > 0);
            _actions["Playback/Once"]->setEnabled(count > 0);
            _actions["Playback/PingPong"]->setEnabled(count > 0);
            _actions["Playback/Start"]->setEnabled(count > 0);
            _actions["Playback/End"]->setEnabled(count > 0);
            _actions["Playback/FramePrev"]->setEnabled(count > 0);
            _actions["Playback/FramePrevX10"]->setEnabled(count > 0);
            _actions["Playback/FramePrevX100"]->setEnabled(count > 0);
            _actions["Playback/FrameNext"]->setEnabled(count > 0);
            _actions["Playback/FrameNextX10"]->setEnabled(count > 0);
            _actions["Playback/FrameNextX100"]->setEnabled(count > 0);
            _actions["Playback/SetInPoint"]->setEnabled(count > 0);
            _actions["Playback/ResetInPoint"]->setEnabled(count > 0);
            _actions["Playback/SetOutPoint"]->setEnabled(count > 0);
            _actions["Playback/ResetOutPoint"]->setEnabled(count > 0);
            _actions["Playback/FocusCurrentFrame"]->setEnabled(count > 0);

            _actions["Audio/IncreaseVolume"]->setEnabled(count > 0);
            _actions["Audio/DecreaseVolume"]->setEnabled(count > 0);
            _actions["Audio/Mute"]->setEnabled(count > 0);

            std::vector<std::string> info;

            if (!_timelinePlayers.empty())
            {
                {
                    QSignalBlocker blocker(_channelsActionGroup);
                    _actions["Image/RedChannel"]->setChecked(false);
                    _actions["Image/GreenChannel"]->setChecked(false);
                    _actions["Image/BlueChannel"]->setChecked(false);
                    _actions["Image/AlphaChannel"]->setChecked(false);
                    if (!_imageOptions.empty())
                    {
                        auto channelsAction = _channelsToActions.find(_imageOptions[0].channels);
                        if (channelsAction != _channelsToActions.end())
                        {
                            channelsAction.value()->setChecked(true);
                        }
                    }
                }
                {
                    QSignalBlocker blocker(_actions["Image/MirrorX"]);
                    _actions["Image/MirrorX"]->setChecked(false);
                    if (!_imageOptions.empty())
                    {
                        _actions["Image/MirrorX"]->setChecked(_imageOptions[0].mirror.x);
                    }
                }
                {
                    QSignalBlocker blocker(_actions["Image/MirrorY"]);
                    _actions["Image/MirrorY"]->setChecked(false);
                    if (!_imageOptions.empty())
                    {
                        _actions["Image/MirrorY"]->setChecked(_imageOptions[0].mirror.y);
                    }
                }
                {
                    QSignalBlocker blocker(_playbackActionGroup);
                    auto playbackAction = _playbackToActions.find(_timelinePlayers[0]->playback());
                    if (playbackAction != _playbackToActions.end())
                    {
                        playbackAction.value()->setChecked(true);
                    }
                }
                {
                    QSignalBlocker blocker(_loopActionGroup);
                    auto loopAction = _loopToActions.find(_timelinePlayers[0]->loop());
                    if (loopAction != _loopToActions.end())
                    {
                        loopAction.value()->setChecked(true);
                    }
                }
                {
                    QSignalBlocker blocker(_actions["Audio/Mute"]);
                    _actions["Audio/Mute"]->setChecked(_timelinePlayers[0]->isMuted());
                }

                const auto& avInfo = _timelinePlayers[0]->avInfo();
                if (!avInfo.video.empty())
                {
                    std::stringstream ss;
                    ss << "Video: " << avInfo.video[0];
                    info.push_back(ss.str());
                }
                if (avInfo.audio.isValid())
                {
                    std::stringstream ss;
                    ss << "Audio: " << avInfo.audio;
                    info.push_back(ss.str());
                }
            }
            else
            {
                {
                    QSignalBlocker blocker(_channelsActionGroup);
                    _actions["Image/RedChannel"]->setChecked(false);
                    _actions["Image/GreenChannel"]->setChecked(false);
                    _actions["Image/BlueChannel"]->setChecked(false);
                    _actions["Image/AlphaChannel"]->setChecked(false);
                }
                {
                    QSignalBlocker blocker(_actions["Image/MirrorX"]);
                    _actions["Image/MirrorX"]->setChecked(false);
                }
                {
                    QSignalBlocker blocker(_actions["Image/MirrorY"]);
                    _actions["Image/MirrorY"]->setChecked(false);
                }
                {
                    QSignalBlocker blocker(_playbackActionGroup);
                    _actions["Playback/Stop"]->setChecked(true);
                }
                {
                    QSignalBlocker blocker(_loopActionGroup);
                    _actions["Playback/Loop"]->setChecked(true);
                }
                {
                    QSignalBlocker blocker(_actions["Audio/Mute"]);
                    _actions["Audio/Mute"]->setChecked(false);
                }
            }

            _timelineWidget->setTimelinePlayers(_timelinePlayers);
            _timelineWidget->setColorConfig(_colorConfig);
            _timelineWidget->setImageOptions(_imageOptions);
            _timelineWidget->setCompareOptions(_compareOptions);

            _compareTool->setCompareOptions(_compareOptions);

            _imageTool->setImageOptions(!_imageOptions.empty() ? _imageOptions[0] : render::ImageOptions());

            _infoTool->setInfo(!_timelinePlayers.empty() ? _timelinePlayers[0]->avInfo() : avio::Info());

            _audioTool->setAudioOffset(!_timelinePlayers.empty() ? _timelinePlayers[0]->audioOffset() : 0.0);

            _infoLabel->setText(QString::fromUtf8(string::join(info, " ").c_str()));

            if (_secondaryWindow)
            {
                _secondaryWindow->setTimelinePlayers(_timelinePlayers);
                _secondaryWindow->setColorConfig(_colorConfig);
                _secondaryWindow->setImageOptions(_imageOptions);
                _secondaryWindow->setCompareOptions(_compareOptions);
            }
        }
    }
}
