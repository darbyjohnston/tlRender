// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2022 Darby Johnston
// All rights reserved.

#include <tlAppPlay/MainWindow.h>

#include <tlAppPlay/App.h>
#include <tlAppPlay/AudioActions.h>
#include <tlAppPlay/AudioTool.h>
#include <tlAppPlay/ColorModel.h>
#include <tlAppPlay/ColorTool.h>
#include <tlAppPlay/CompareActions.h>
#include <tlAppPlay/CompareTool.h>
#include <tlAppPlay/FileActions.h>
#include <tlAppPlay/FilesModel.h>
#include <tlAppPlay/FilesTool.h>
#include <tlAppPlay/ImageActions.h>
#include <tlAppPlay/InfoTool.h>
#include <tlAppPlay/MessagesTool.h>
#include <tlAppPlay/PlaybackActions.h>
#include <tlAppPlay/SecondaryWindow.h>
#include <tlAppPlay/SettingsObject.h>
#include <tlAppPlay/SettingsTool.h>
#include <tlAppPlay/SystemLogTool.h>
#include <tlAppPlay/ViewActions.h>
#include <tlAppPlay/WindowActions.h>

#include <tlQtWidget/Spacer.h>
#include <tlQtWidget/TimeLabel.h>
#include <tlQtWidget/TimeSpinBox.h>
#include <tlQtWidget/TimelineSlider.h>
#include <tlQtWidget/TimelineViewport.h>
#include <tlQtWidget/Util.h>

#include <tlCore/File.h>
#include <tlCore/String.h>
#include <tlCore/StringFormat.h>

#include <QComboBox>
#include <QDockWidget>
#include <QDoubleSpinBox>
#include <QDragEnterEvent>
#include <QDragMoveEvent>
#include <QHBoxLayout>
#include <QLabel>
#include <QMenuBar>
#include <QMimeData>
#include <QMouseEvent>
#include <QStatusBar>
#include <QStyle>
#include <QToolBar>

namespace tl
{
    namespace play
    {
        namespace
        {
            const size_t sliderSteps = 100;
            const size_t errorTimeout = 5000;
        }

        struct MainWindow::Private
        {
            App* app = nullptr;

            std::vector<qt::TimelinePlayer*> timelinePlayers;
            bool floatOnTop = false;
            bool secondaryFloatOnTop = false;
            imaging::ColorConfig colorConfig;
            timeline::ImageOptions imageOptions;
            timeline::CompareOptions compareOptions;

            FileActions* fileActions = nullptr;
            CompareActions* compareActions = nullptr;
            WindowActions* windowActions = nullptr;
            ViewActions* viewActions = nullptr;
            ImageActions* imageActions = nullptr;
            PlaybackActions* playbackActions = nullptr;
            AudioActions* audioActions = nullptr;

            QComboBox* filesComboBox = nullptr;
            QComboBox* filesBComboBox = nullptr;
            qtwidget::TimelineViewport* timelineViewport = nullptr;
            qtwidget::TimelineSlider* timelineSlider = nullptr;
            qtwidget::TimeSpinBox* currentTimeSpinBox = nullptr;
            qtwidget::TimeLabel* durationLabel = nullptr;
            QDoubleSpinBox* speedSpinBox = nullptr;
            QSlider* volumeSlider = nullptr;
            FilesTool* filesTool = nullptr;
            CompareTool* compareTool = nullptr;
            ColorTool* colorTool = nullptr;
            InfoTool* infoTool = nullptr;
            AudioTool* audioTool = nullptr;
            SettingsTool* settingsTool = nullptr;
            MessagesTool* messagesTool = nullptr;
            SystemLogTool* systemLogTool = nullptr;
            QLabel* infoLabel = nullptr;
            QStatusBar* statusBar = nullptr;
            SecondaryWindow* secondaryWindow = nullptr;

            std::shared_ptr<observer::ListObserver<std::shared_ptr<FilesModelItem> > > filesObserver;
            std::shared_ptr<observer::ValueObserver<int> > aIndexObserver;
            std::shared_ptr<observer::ListObserver<int> > bIndexesObserver;
            std::shared_ptr<observer::ListObserver<timeline::ImageOptions> > imageOptionsObserver;
            std::shared_ptr<observer::ValueObserver<timeline::CompareOptions> > compareOptionsObserver;
            std::shared_ptr<observer::ValueObserver<imaging::ColorConfig> > colorConfigObserver;
            std::shared_ptr<observer::ValueObserver<log::Item> > logObserver;

            bool mousePressed = false;
            math::Vector2i mousePos;
        };

        MainWindow::MainWindow(App* app, QWidget* parent) :
            QMainWindow(parent),
            _p(new Private)
        {
            TLRENDER_P();

            p.app = app;
            p.imageOptions = app->imageOptions();

            setFocusPolicy(Qt::ClickFocus);
            setAcceptDrops(true);

            p.fileActions = new FileActions(app, this);
            p.compareActions = new CompareActions(app, this);
            p.windowActions = new WindowActions(app, this);
            p.viewActions = new ViewActions(app, this);
            p.imageActions = new ImageActions(app, this);
            p.playbackActions = new PlaybackActions(app, this);
            p.audioActions = new AudioActions(app, this);

            auto menuBar = new QMenuBar;
            menuBar->addMenu(p.fileActions->menu());
            menuBar->addMenu(p.compareActions->menu());
            menuBar->addMenu(p.windowActions->menu());
            menuBar->addMenu(p.viewActions->menu());
            menuBar->addMenu(p.imageActions->menu());
            menuBar->addMenu(p.playbackActions->menu());
            menuBar->addMenu(p.audioActions->menu());
            setMenuBar(menuBar);

            p.filesComboBox = new QComboBox;
            p.filesComboBox->setMinimumContentsLength(10);
            p.filesComboBox->setToolTip(tr("Set the current file"));
            p.filesBComboBox = new QComboBox;
            p.filesBComboBox->setMinimumContentsLength(10);
            p.filesBComboBox->setToolTip(tr("Set the B file"));
            auto topToolBar = new QToolBar;
            topToolBar->setObjectName("TopToolBar");
            topToolBar->setWindowTitle(tr("Top ToolBar"));
            topToolBar->setIconSize(QSize(20, 20));
            topToolBar->setAllowedAreas(Qt::TopToolBarArea);
            topToolBar->setFloatable(false);
            topToolBar->setMovable(false);
            topToolBar->addWidget(p.filesComboBox);
            topToolBar->addAction(p.fileActions->actions()["Open"]);
            topToolBar->addAction(p.fileActions->actions()["OpenWithAudio"]);
            topToolBar->addAction(p.fileActions->actions()["Close"]);
            topToolBar->addAction(p.fileActions->actions()["CloseAll"]);
            topToolBar->addWidget(new qtwidget::Spacer(Qt::Horizontal));
            topToolBar->addWidget(new qtwidget::Spacer(Qt::Horizontal));
            topToolBar->addWidget(p.filesBComboBox);
            topToolBar->addAction(p.compareActions->actions()["A"]);
            topToolBar->addAction(p.compareActions->actions()["B"]);
            topToolBar->addAction(p.compareActions->actions()["Wipe"]);
            topToolBar->addAction(p.compareActions->actions()["Tile"]);
            topToolBar->addWidget(new qtwidget::Spacer(Qt::Horizontal));
            topToolBar->addWidget(new qtwidget::Spacer(Qt::Horizontal));
            topToolBar->addAction(p.windowActions->actions()["FullScreen"]);
            topToolBar->addAction(p.windowActions->actions()["Secondary"]);
            topToolBar->addWidget(new qtwidget::Spacer(Qt::Horizontal));
            topToolBar->addWidget(new qtwidget::Spacer(Qt::Horizontal));
            topToolBar->addAction(p.viewActions->actions()["Frame"]);
            topToolBar->addAction(p.viewActions->actions()["Zoom1To1"]);
            addToolBar(Qt::TopToolBarArea, topToolBar);

            p.timelineViewport = new qtwidget::TimelineViewport(app->getContext());
            p.timelineViewport->installEventFilter(this);
            setCentralWidget(p.timelineViewport);

            p.timelineSlider = new qtwidget::TimelineSlider(app->getContext());
            p.timelineSlider->setTimeObject(app->timeObject());
            p.timelineSlider->setThumbnails(app->settingsObject()->value("Timeline/Thumbnails").toBool());
            auto timelineDockWidget = new QDockWidget;
            timelineDockWidget->setObjectName("Timeline");
            timelineDockWidget->setWindowTitle(tr("Timeline"));
            timelineDockWidget->setFeatures(QDockWidget::DockWidgetClosable);
            timelineDockWidget->setTitleBarWidget(new QWidget);
            timelineDockWidget->setWidget(p.timelineSlider);
            addDockWidget(Qt::BottomDockWidgetArea, timelineDockWidget);

            p.currentTimeSpinBox = new qtwidget::TimeSpinBox;
            p.currentTimeSpinBox->setTimeObject(app->timeObject());
            p.currentTimeSpinBox->setToolTip(tr("Current time"));
            p.durationLabel = new qtwidget::TimeLabel;
            p.durationLabel->setTimeObject(app->timeObject());
            p.durationLabel->setToolTip(tr("Timeline duration"));
            p.speedSpinBox = new QDoubleSpinBox;
            p.speedSpinBox->setRange(0.0, 120.0);
            p.speedSpinBox->setSingleStep(1.0);
            const QFont fixedFont = qtwidget::font("NotoMono-Regular");
            p.speedSpinBox->setFont(fixedFont);
            p.speedSpinBox->setToolTip(tr("Timeline speed (frames per second)"));
            p.volumeSlider = new QSlider(Qt::Horizontal);
            p.volumeSlider->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);
            p.volumeSlider->setToolTip(tr("Audio volume"));
            auto bottomToolBar = new QToolBar;
            bottomToolBar->setObjectName("BottomToolBar");
            bottomToolBar->setWindowTitle(tr("Bottom ToolBar"));
            bottomToolBar->setIconSize(QSize(20, 20));
            bottomToolBar->setAllowedAreas(Qt::BottomToolBarArea);
            bottomToolBar->setFloatable(false);
            bottomToolBar->setMovable(false);
            bottomToolBar->addAction(p.playbackActions->actions()["Reverse"]);
            bottomToolBar->addAction(p.playbackActions->actions()["Stop"]);
            bottomToolBar->addAction(p.playbackActions->actions()["Forward"]);
            bottomToolBar->addAction(p.playbackActions->actions()["Start"]);
            bottomToolBar->addAction(p.playbackActions->actions()["FramePrev"]);
            bottomToolBar->addAction(p.playbackActions->actions()["FrameNext"]);
            bottomToolBar->addAction(p.playbackActions->actions()["End"]);
            bottomToolBar->addWidget(new qtwidget::Spacer(Qt::Horizontal));
            bottomToolBar->addWidget(new qtwidget::Spacer(Qt::Horizontal));
            bottomToolBar->addWidget(p.currentTimeSpinBox);
            bottomToolBar->addWidget(new qtwidget::Spacer(Qt::Horizontal));
            bottomToolBar->addWidget(p.durationLabel);
            bottomToolBar->addWidget(new qtwidget::Spacer(Qt::Horizontal));
            bottomToolBar->addWidget(p.speedSpinBox);
            bottomToolBar->addAction(p.playbackActions->actions()["Speed/Default"]);
            bottomToolBar->addWidget(new qtwidget::Spacer(Qt::Horizontal));
            bottomToolBar->addWidget(new qtwidget::Spacer(Qt::Horizontal));
            bottomToolBar->addAction(p.audioActions->actions()["Mute"]);
            bottomToolBar->addWidget(p.volumeSlider);
            addToolBar(Qt::BottomToolBarArea, bottomToolBar);

            p.windowActions->menu()->addSeparator();
            p.windowActions->menu()->addAction(topToolBar->toggleViewAction());
            p.windowActions->menu()->addAction(timelineDockWidget->toggleViewAction());
            p.windowActions->menu()->addAction(bottomToolBar->toggleViewAction());

            p.filesTool = new FilesTool(p.fileActions->actions(), app);
            auto fileDockWidget = new QDockWidget;
            fileDockWidget->setObjectName("FilesTool");
            fileDockWidget->setWindowTitle(tr("Files"));
            fileDockWidget->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);
            fileDockWidget->setWidget(p.filesTool);
            fileDockWidget->hide();
            fileDockWidget->toggleViewAction()->setShortcut(QKeySequence(Qt::Key_F1));
            p.windowActions->menu()->addSeparator();
            p.windowActions->menu()->addAction(fileDockWidget->toggleViewAction());
            addDockWidget(Qt::RightDockWidgetArea, fileDockWidget);

            p.compareTool = new CompareTool(p.compareActions->actions(), app);
            auto compareDockWidget = new QDockWidget;
            compareDockWidget->setObjectName("CompareTool");
            compareDockWidget->setWindowTitle(tr("Compare"));
            compareDockWidget->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);
            compareDockWidget->setWidget(p.compareTool);
            compareDockWidget->hide();
            compareDockWidget->toggleViewAction()->setShortcut(QKeySequence(Qt::Key_F2));
            p.windowActions->menu()->addAction(compareDockWidget->toggleViewAction());
            addDockWidget(Qt::RightDockWidgetArea, compareDockWidget);

            p.colorTool = new ColorTool(app->colorModel());
            auto colorDockWidget = new QDockWidget;
            colorDockWidget->setObjectName("ColorTool");
            colorDockWidget->setWindowTitle(tr("Color"));
            colorDockWidget->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);
            colorDockWidget->setWidget(p.colorTool);
            colorDockWidget->hide();
            colorDockWidget->toggleViewAction()->setShortcut(QKeySequence(Qt::Key_F3));
            p.windowActions->menu()->addAction(colorDockWidget->toggleViewAction());
            addDockWidget(Qt::RightDockWidgetArea, colorDockWidget);

            p.infoTool = new InfoTool(app);
            auto infoDockWidget = new QDockWidget;
            infoDockWidget->setObjectName("InfoTool");
            infoDockWidget->setWindowTitle(tr("Information"));
            infoDockWidget->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);
            infoDockWidget->setWidget(p.infoTool);
            infoDockWidget->hide();
            infoDockWidget->toggleViewAction()->setShortcut(QKeySequence(Qt::Key_F4));
            p.windowActions->menu()->addAction(infoDockWidget->toggleViewAction());
            addDockWidget(Qt::RightDockWidgetArea, infoDockWidget);

            p.audioTool = new AudioTool();
            auto audioDockWidget = new QDockWidget;
            audioDockWidget->setObjectName("AudioTool");
            audioDockWidget->setWindowTitle(tr("Audio"));
            audioDockWidget->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);
            audioDockWidget->setWidget(p.audioTool);
            audioDockWidget->hide();
            audioDockWidget->toggleViewAction()->setShortcut(QKeySequence(Qt::Key_F5));
            p.windowActions->menu()->addAction(audioDockWidget->toggleViewAction());
            addDockWidget(Qt::RightDockWidgetArea, audioDockWidget);

            p.settingsTool = new SettingsTool(app->settingsObject(), app->timeObject());
            auto settingsDockWidget = new QDockWidget;
            settingsDockWidget->setObjectName("SettingsTool");
            settingsDockWidget->setWindowTitle(tr("Settings"));
            settingsDockWidget->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);
            settingsDockWidget->setWidget(p.settingsTool);
            settingsDockWidget->hide();
            settingsDockWidget->toggleViewAction()->setShortcut(QKeySequence(Qt::Key_F9));
            p.windowActions->menu()->addAction(settingsDockWidget->toggleViewAction());
            addDockWidget(Qt::RightDockWidgetArea, settingsDockWidget);

            p.messagesTool = new MessagesTool(app->getContext());
            auto messagesDockWidget = new QDockWidget;
            messagesDockWidget->setObjectName("MessagesTool");
            messagesDockWidget->setWindowTitle(tr("Messages"));
            messagesDockWidget->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);
            messagesDockWidget->setWidget(p.messagesTool);
            messagesDockWidget->hide();
            messagesDockWidget->toggleViewAction()->setShortcut(QKeySequence(Qt::Key_F10));
            p.windowActions->menu()->addAction(messagesDockWidget->toggleViewAction());
            addDockWidget(Qt::RightDockWidgetArea, messagesDockWidget);

            p.systemLogTool = new SystemLogTool(app->getContext());
            auto systemLogDockWidget = new QDockWidget;
            systemLogDockWidget->setObjectName("SystemLogTool");
            systemLogDockWidget->setWindowTitle(tr("System Log"));
            systemLogDockWidget->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);
            systemLogDockWidget->setWidget(p.systemLogTool);
            systemLogDockWidget->hide();
            systemLogDockWidget->toggleViewAction()->setShortcut(QKeySequence(Qt::Key_F11));
            p.windowActions->menu()->addAction(systemLogDockWidget->toggleViewAction());
            addDockWidget(Qt::RightDockWidgetArea, systemLogDockWidget);

            p.infoLabel = new QLabel;

            p.statusBar = new QStatusBar;
            p.statusBar->addPermanentWidget(p.infoLabel);
            setStatusBar(p.statusBar);

            _widgetUpdate();

            p.filesObserver = observer::ListObserver<std::shared_ptr<FilesModelItem> >::create(
                app->filesModel()->observeFiles(),
                [this](const std::vector<std::shared_ptr<FilesModelItem> >&)
                {
                    _widgetUpdate();
                });
            p.aIndexObserver = observer::ValueObserver<int>::create(
                app->filesModel()->observeAIndex(),
                [this](int)
                {
                    _widgetUpdate();
                });
            p.bIndexesObserver = observer::ListObserver<int>::create(
                app->filesModel()->observeBIndexes(),
                [this](const std::vector<int>&)
                {
                    _widgetUpdate();
                });
            p.compareOptionsObserver = observer::ValueObserver<timeline::CompareOptions>::create(
                app->filesModel()->observeCompareOptions(),
                [this](const timeline::CompareOptions& value)
                {
                    _p->compareOptions = value;
                    _widgetUpdate();
                });

            p.colorConfigObserver = observer::ValueObserver<imaging::ColorConfig>::create(
                app->colorModel()->observeConfig(),
                [this](const imaging::ColorConfig& value)
                {
                    _p->colorConfig = value;
                    _widgetUpdate();
                });

            p.logObserver = observer::ValueObserver<log::Item>::create(
                app->getContext()->getLogSystem()->observeLog(),
                [this](const log::Item& value)
                {
                    switch (value.type)
                    {
                    case log::Type::Error:
                        _p->statusBar->showMessage(
                            QString(tr("ERROR: %1")).
                            arg(QString::fromUtf8(value.message.c_str())),
                            errorTimeout);
                        break;
                    default: break;
                    }
                });

            connect(
                app,
                &App::imageOptionsChanged,
                [this](const timeline::ImageOptions& value)
                {
                    _p->imageOptions = value;
                    _widgetUpdate();
                });

            connect(
                p.windowActions,
                &WindowActions::resize,
                [this](const imaging::Size& size)
                {
                    resize(size.w, size.h);
                });
            connect(
                p.windowActions->actions()["FullScreen"],
                &QAction::triggered,
                [this]
                {
                    setWindowState(windowState() ^ Qt::WindowFullScreen);
                });
            connect(
                p.windowActions->actions()["FloatOnTop"],
                &QAction::toggled,
                [this](bool value)
                {
                    _p->floatOnTop = value;
                    if (_p->floatOnTop)
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
                p.windowActions->actions()["Secondary"],
                SIGNAL(toggled(bool)),
                SLOT(_secondaryWindowCallback(bool)));
            connect(
                p.windowActions->actions()["SecondaryFloatOnTop"],
                &QAction::toggled,
                [this](bool value)
                {
                    _p->secondaryFloatOnTop = value;
                    if (_p->secondaryWindow)
                    {
                        if (_p->secondaryFloatOnTop)
                        {
                            _p->secondaryWindow->setWindowFlags(_p->secondaryWindow->windowFlags() | Qt::WindowStaysOnTopHint);
                        }
                        else
                        {
                            _p->secondaryWindow->setWindowFlags(_p->secondaryWindow->windowFlags() & ~Qt::WindowStaysOnTopHint);
                        }
                        _p->secondaryWindow->show();
                    }
                });

            connect(
                p.viewActions->actions()["Frame"],
                &QAction::triggered,
                [this]
                {
                    _p->timelineViewport->frameView();
                });
            connect(
                p.viewActions->actions()["Zoom1To1"],
                &QAction::triggered,
                [this]
                {
                    _p->timelineViewport->viewZoom1To1();
                });
            connect(
                p.viewActions->actions()["ZoomIn"],
                &QAction::triggered,
                [this]
                {
                    _p->timelineViewport->viewZoomIn();
                });
            connect(
                p.viewActions->actions()["ZoomOut"],
                &QAction::triggered,
                [this]
                {
                    _p->timelineViewport->viewZoomOut();
                });

            connect(
                p.playbackActions->actions()["FocusCurrentFrame"],
                &QAction::triggered,
                [this]
                {
                    _p->currentTimeSpinBox->setFocus(Qt::OtherFocusReason);
                    _p->currentTimeSpinBox->selectAll();
                });

            connect(
                p.filesComboBox,
                QOverload<int>::of(&QComboBox::activated),
                [app](int value)
                {
                    app->filesModel()->setA(value);
                });

            connect(
                p.filesBComboBox,
                QOverload<int>::of(&QComboBox::activated),
                [app](int value)
                {
                    app->filesModel()->clearB();
                    app->filesModel()->setB(value, true);
                });

            connect(
                p.currentTimeSpinBox,
                &qtwidget::TimeSpinBox::valueChanged,
                [this](const otime::RationalTime& value)
                {
                    if (!_p->timelinePlayers.empty())
                    {
                        _p->timelinePlayers[0]->setPlayback(timeline::Playback::Stop);
                        _p->timelinePlayers[0]->seek(value);
                    }
                });

            connect(
                p.speedSpinBox,
                QOverload<double>::of(&QDoubleSpinBox::valueChanged),
                [this](double value)
                {
                    if (!_p->timelinePlayers.empty())
                    {
                        _p->timelinePlayers[0]->setSpeed(value);
                    }
                });

            connect(
                p.volumeSlider,
                SIGNAL(valueChanged(int)),
                SLOT(_volumeCallback(int)));

            connect(
                p.compareTool,
                &CompareTool::compareOptionsChanged,
                [app](const timeline::CompareOptions& value)
                {
                    app->filesModel()->setCompareOptions(value);
                });

            connect(
                p.colorTool,
                &ColorTool::imageOptionsChanged,
                [app](const timeline::ImageOptions& value)
                {
                    app->setImageOptions(value);
                });

            connect(
                p.audioTool,
                &AudioTool::audioOffsetChanged,
                [this](double value)
                {
                    if (!_p->timelinePlayers.empty())
                    {
                        _p->timelinePlayers[0]->setAudioOffset(value);
                    }
                });

            connect(
                app->settingsObject(),
                &SettingsObject::valueChanged,
                [this](const QString& name, const QVariant& value)
                {
                    if ("Timeline/Thumbnails" == name)
                    {
                        _p->timelineSlider->setThumbnails(value.toBool());
                    }
                });

            app->settingsObject()->setDefaultValue("MainWindow/geometry", QByteArray());
            auto ba = app->settingsObject()->value("MainWindow/geometry").toByteArray();
            if (!ba.isEmpty())
            {
                restoreGeometry(ba);
            }
            else
            {
                resize(1280, 720);
            }
            app->settingsObject()->setDefaultValue("MainWindow/windowState", QByteArray());
            ba = app->settingsObject()->value("MainWindow/windowState").toByteArray();
            if (!ba.isEmpty())
            {
                restoreState(ba);
            }
            app->settingsObject()->setDefaultValue("MainWindow/FloatOnTop", false);
            p.floatOnTop = app->settingsObject()->value("MainWindow/FloatOnTop").toBool();
            if (p.floatOnTop)
            {
                setWindowFlags(windowFlags() | Qt::WindowStaysOnTopHint);
            }
            else
            {
                setWindowFlags(windowFlags() & ~Qt::WindowStaysOnTopHint);
            }
            {
                QSignalBlocker blocker(p.windowActions->actions()["FloatOnTop"]);
                p.windowActions->actions()["FloatOnTop"]->setChecked(p.floatOnTop);
            }
            app->settingsObject()->setDefaultValue("MainWindow/SecondaryFloatOnTop", false);
            p.secondaryFloatOnTop = app->settingsObject()->value("MainWindow/SecondaryFloatOnTop").toBool();
            {
                QSignalBlocker blocker(p.windowActions->actions()["SecondaryFloatOnTop"]);
                p.windowActions->actions()["SecondaryFloatOnTop"]->setChecked(p.secondaryFloatOnTop);
            }
        }

        MainWindow::~MainWindow()
        {
            TLRENDER_P();
            p.app->settingsObject()->setValue("MainWindow/geometry", saveGeometry());
            p.app->settingsObject()->setValue("MainWindow/windowState", saveState());
            p.app->settingsObject()->setValue("MainWindow/FloatOnTop", p.floatOnTop);
            p.app->settingsObject()->setValue("MainWindow/SecondaryFloatOnTop", p.secondaryFloatOnTop);
            if (p.secondaryWindow)
            {
                delete p.secondaryWindow;
                p.secondaryWindow = nullptr;
            }
        }

        void MainWindow::setTimelinePlayers(const std::vector<qt::TimelinePlayer*>& timelinePlayers)
        {
            TLRENDER_P();
            if (!p.timelinePlayers.empty())
            {
                disconnect(
                    p.timelinePlayers[0],
                    SIGNAL(speedChanged(double)),
                    this,
                    SLOT(_speedCallback(double)));
                disconnect(
                    p.timelinePlayers[0],
                    SIGNAL(playbackChanged(tl::timeline::Playback)),
                    this,
                    SLOT(_playbackCallback(tl::timeline::Playback)));
                disconnect(
                    p.timelinePlayers[0],
                    SIGNAL(currentTimeChanged(const otime::RationalTime&)),
                    this,
                    SLOT(_currentTimeCallback(const otime::RationalTime&)));
                disconnect(
                    p.timelinePlayers[0],
                    SIGNAL(volumeChanged(float)),
                    this,
                    SLOT(_volumeCallback(float)));
                disconnect(
                    p.timelinePlayers[0],
                    SIGNAL(audioOffsetChanged(double)),
                    p.audioTool,
                    SLOT(setAudioOffset(double)));
            }

            p.timelinePlayers = timelinePlayers;

            if (!p.timelinePlayers.empty())
            {
                connect(
                    p.timelinePlayers[0],
                    SIGNAL(speedChanged(double)),
                    SLOT(_speedCallback(double)));
                connect(
                    p.timelinePlayers[0],
                    SIGNAL(playbackChanged(tl::timeline::Playback)),
                    SLOT(_playbackCallback(tl::timeline::Playback)));
                connect(
                    p.timelinePlayers[0],
                    SIGNAL(currentTimeChanged(const otime::RationalTime&)),
                    SLOT(_currentTimeCallback(const otime::RationalTime&)));
                connect(
                    p.timelinePlayers[0],
                    SIGNAL(volumeChanged(float)),
                    SLOT(_volumeCallback(float)));
                connect(
                    p.timelinePlayers[0],
                    SIGNAL(audioOffsetChanged(double)),
                    p.audioTool,
                    SLOT(setAudioOffset(double)));
            }

            _widgetUpdate();
        }

        void MainWindow::closeEvent(QCloseEvent*)
        {
            TLRENDER_P();
            if (p.secondaryWindow)
            {
                delete p.secondaryWindow;
                p.secondaryWindow = nullptr;
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
            TLRENDER_P();
            const QMimeData* mimeData = event->mimeData();
            if (mimeData->hasUrls())
            {
                const auto urlList = mimeData->urls();
                for (int i = 0; i < urlList.size(); ++i)
                {
                    const QString fileName = urlList[i].toLocalFile();
                    p.app->open(fileName);
                }
            }
        }

        bool MainWindow::eventFilter(QObject* obj, QEvent* event)
        {
            TLRENDER_P();
            bool out = QObject::eventFilter(obj, event);;
            if (p.timelineViewport == obj)
            {
                if (event->type() == QEvent::Enter)
                {
                    p.mousePressed = false;
                }
                else if (event->type() == QEvent::Leave)
                {
                    p.mousePressed = false;
                }
                else if (event->type() == QEvent::MouseButtonPress)
                {
                    auto mouseEvent = static_cast<QMouseEvent*>(event);
                    if (Qt::LeftButton == mouseEvent->button() && mouseEvent->modifiers() & Qt::AltModifier)
                    {
                        p.mousePressed = true;
                        out = true;
                    }
                }
                else if (event->type() == QEvent::MouseButtonRelease)
                {
                    if (p.mousePressed)
                    {
                        p.mousePressed = false;
                        out = true;
                    }
                }
                else if (event->type() == QEvent::MouseMove)
                {
                    auto mouseEvent = static_cast<QMouseEvent*>(event);
                    p.mousePos.x = mouseEvent->x();
                    p.mousePos.y = p.timelineViewport->height() - 1 - mouseEvent->y();
                    if (p.mousePressed)
                    {
                        if (!p.timelinePlayers.empty())
                        {
                            const auto& ioInfo = p.timelinePlayers[0]->ioInfo();
                            if (!ioInfo.video.empty())
                            {
                                const auto& imageInfo = ioInfo.video[0];
                                const math::Vector2i& viewPos = p.timelineViewport->viewPos();
                                const float viewZoom = p.timelineViewport->viewZoom();
                                auto compareOptions = p.compareOptions;
                                compareOptions.wipeCenter.x = (p.mousePos.x - viewPos.x) / viewZoom /
                                    static_cast<float>(imageInfo.size.w);
                                compareOptions.wipeCenter.y = 1.F - (p.mousePos.y - viewPos.y) / viewZoom /
                                    static_cast<float>(imageInfo.size.h);
                                p.app->filesModel()->setCompareOptions(compareOptions);
                            }
                        }
                        out = true;
                    }
                }
            }
            return out;
        }

        void MainWindow::_secondaryWindowCallback(bool value)
        {
            TLRENDER_P();
            if (value && !p.secondaryWindow)
            {
                p.secondaryWindow = new SecondaryWindow(p.app);
                p.secondaryWindow->viewport()->setColorConfig(p.colorConfig);
                std::vector<timeline::ImageOptions> imageOptions;
                for (const auto& i : p.timelinePlayers)
                {
                    imageOptions.push_back(p.imageOptions);
                }
                p.secondaryWindow->viewport()->setImageOptions(imageOptions);
                p.secondaryWindow->viewport()->setCompareOptions(p.compareOptions);
                p.secondaryWindow->viewport()->setTimelinePlayers(p.timelinePlayers);

                connect(
                    p.timelineViewport,
                    SIGNAL(viewPosAndZoomChanged(const tl::math::Vector2i&, float)),
                    p.secondaryWindow->viewport(),
                    SLOT(setViewPosAndZoom(const tl::math::Vector2i&, float)));
                connect(
                    p.timelineViewport,
                    SIGNAL(frameViewActivated()),
                    p.secondaryWindow->viewport(),
                    SLOT(frameView()));

                connect(
                    p.secondaryWindow,
                    SIGNAL(destroyed(QObject*)),
                    SLOT(_secondaryWindowDestroyedCallback()));

                if (p.secondaryFloatOnTop)
                {
                    p.secondaryWindow->setWindowFlags(p.secondaryWindow->windowFlags() | Qt::WindowStaysOnTopHint);
                }
                else
                {
                    p.secondaryWindow->setWindowFlags(p.secondaryWindow->windowFlags() & ~Qt::WindowStaysOnTopHint);
                }
                p.secondaryWindow->show();
            }
            else if (!value && p.secondaryWindow)
            {
                delete p.secondaryWindow;
                p.secondaryWindow = nullptr;
            }
        }

        void MainWindow::_secondaryWindowDestroyedCallback()
        {
            TLRENDER_P();
            p.secondaryWindow = nullptr;
            p.windowActions->actions()["Secondary"]->setChecked(false);
        }

        void MainWindow::_speedCallback(double)
        {
            _widgetUpdate();
        }

        void MainWindow::_playbackCallback(timeline::Playback)
        {
            _widgetUpdate();
        }

        void MainWindow::_currentTimeCallback(const otime::RationalTime& value)
        {
            TLRENDER_P();
            const QSignalBlocker blocker(p.currentTimeSpinBox);
            p.currentTimeSpinBox->setValue(value);
        }

        void MainWindow::_volumeCallback(int value)
        {
            TLRENDER_P();
            if (!p.timelinePlayers.empty())
            {
                p.timelinePlayers[0]->setVolume(value / static_cast<float>(sliderSteps));
            }
        }

        void MainWindow::_volumeCallback(float value)
        {
            TLRENDER_P();
            const QSignalBlocker blocker(p.volumeSlider);
            p.volumeSlider->setValue(value * sliderSteps);
        }

        void MainWindow::_widgetUpdate()
        {
            TLRENDER_P();

            const int count = p.app->filesModel()->observeFiles()->getSize();
            p.timelineSlider->setEnabled(count > 0);
            p.currentTimeSpinBox->setEnabled(count > 0);
            p.speedSpinBox->setEnabled(count > 0);
            p.volumeSlider->setEnabled(count > 0);

            std::vector<std::string> info;

            if (!p.timelinePlayers.empty())
            {
                {
                    QSignalBlocker blocker(p.filesComboBox);
                    p.filesComboBox->clear();
                    for (const auto& i : p.app->filesModel()->observeFiles()->get())
                    {
                        p.filesComboBox->addItem(QString::fromUtf8(i->path.get(-1, false).c_str()));
                    }
                    p.filesComboBox->setCurrentIndex(p.app->filesModel()->observeAIndex()->get());
                }

                {
                    QSignalBlocker blocker(p.filesBComboBox);
                    p.filesBComboBox->clear();
                    for (const auto& i : p.app->filesModel()->observeFiles()->get())
                    {
                        p.filesBComboBox->addItem(QString::fromUtf8(i->path.get(-1, false).c_str()));
                    }
                    int index = 0;
                    const auto& indexes = p.app->filesModel()->observeBIndexes()->get();
                    if (!indexes.empty())
                    {
                        index = indexes[0];
                    }
                    p.filesBComboBox->setCurrentIndex(index);
                }

                {
                    QSignalBlocker blocker(p.speedSpinBox);
                    p.speedSpinBox->setValue(p.timelinePlayers[0]->speed());
                }

                const auto& duration = p.timelinePlayers[0]->duration();
                p.durationLabel->setValue(duration);

                {
                    QSignalBlocker blocker(p.volumeSlider);
                    p.volumeSlider->setValue(p.timelinePlayers[0]->volume() * sliderSteps);
                }

                const auto& ioInfo = p.timelinePlayers[0]->ioInfo();
                if (!ioInfo.video.empty())
                {
                    std::stringstream ss;
                    ss << "Video: " << ioInfo.video[0];
                    info.push_back(ss.str());
                }
                if (ioInfo.audio.isValid())
                {
                    std::stringstream ss;
                    ss << "Audio: " << ioInfo.audio;
                    info.push_back(ss.str());
                }
            }
            else
            {
                {
                    QSignalBlocker blocker(p.filesComboBox);
                    p.filesComboBox->clear();
                }

                {
                    QSignalBlocker blocker(p.filesBComboBox);
                    p.filesBComboBox->clear();
                }

                {
                    QSignalBlocker blocker(p.speedSpinBox);
                    p.speedSpinBox->setValue(0.0);
                }

                p.durationLabel->setValue(time::invalidTime);

                {
                    QSignalBlocker blocker(p.volumeSlider);
                    p.volumeSlider->setValue(0);
                }
            }

            p.fileActions->setTimelinePlayers(p.timelinePlayers);

            p.compareActions->setCompareOptions(p.compareOptions);
            p.compareActions->setTimelinePlayers(p.timelinePlayers);

            p.windowActions->setTimelinePlayers(p.timelinePlayers);

            p.viewActions->setTimelinePlayers(p.timelinePlayers);

            p.imageActions->setImageOptions(p.imageOptions);
            p.imageActions->setTimelinePlayers(p.timelinePlayers);

            p.playbackActions->setTimelinePlayers(p.timelinePlayers);

            p.audioActions->setTimelinePlayers(p.timelinePlayers);

            p.timelineViewport->setColorConfig(p.colorConfig);
            std::vector<timeline::ImageOptions> imageOptions;
            for (const auto& i : p.timelinePlayers)
            {
                imageOptions.push_back(p.imageOptions);
            }
            p.timelineViewport->setImageOptions(imageOptions);
            p.timelineViewport->setCompareOptions(p.compareOptions);
            p.timelineViewport->setTimelinePlayers(p.timelinePlayers);

            p.timelineSlider->setColorConfig(p.colorConfig);
            p.timelineSlider->setTimelinePlayer(!p.timelinePlayers.empty() ? p.timelinePlayers[0] : nullptr);

            p.compareTool->setCompareOptions(p.compareOptions);

            p.colorTool->setImageOptions(p.imageOptions);

            p.infoTool->setInfo(!p.timelinePlayers.empty() ? p.timelinePlayers[0]->ioInfo() : io::Info());

            p.audioTool->setAudioOffset(!p.timelinePlayers.empty() ? p.timelinePlayers[0]->audioOffset() : 0.0);

            p.infoLabel->setText(QString::fromUtf8(string::join(info, " ").c_str()));

            if (p.secondaryWindow)
            {
                p.secondaryWindow->viewport()->setTimelinePlayers(p.timelinePlayers);
                p.secondaryWindow->viewport()->setColorConfig(p.colorConfig);
                p.secondaryWindow->viewport()->setImageOptions(imageOptions);
                p.secondaryWindow->viewport()->setCompareOptions(p.compareOptions);
            }
        }
    }
}
