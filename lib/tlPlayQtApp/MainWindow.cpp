// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2023 Darby Johnston
// All rights reserved.

#include <tlPlayQtApp/MainWindow.h>

#include <tlPlayQtApp/App.h>
#include <tlPlayQtApp/AudioActions.h>
#include <tlPlayQtApp/AudioTool.h>
#include <tlPlayQtApp/ColorModel.h>
#include <tlPlayQtApp/ColorTool.h>
#include <tlPlayQtApp/CompareActions.h>
#include <tlPlayQtApp/CompareTool.h>
#include <tlPlayQtApp/DevicesModel.h>
#include <tlPlayQtApp/DevicesTool.h>
#include <tlPlayQtApp/FileActions.h>
#include <tlPlayQtApp/FilesTool.h>
#include <tlPlayQtApp/InfoTool.h>
#include <tlPlayQtApp/MessagesTool.h>
#include <tlPlayQtApp/PlaybackActions.h>
#include <tlPlayQtApp/RenderActions.h>
#include <tlPlayQtApp/SecondaryWindow.h>
#include <tlPlayQtApp/SettingsObject.h>
#include <tlPlayQtApp/SettingsTool.h>
#include <tlPlayQtApp/SystemLogTool.h>
#include <tlPlayQtApp/ToolActions.h>
#include <tlPlayQtApp/ViewActions.h>
#include <tlPlayQtApp/WindowActions.h>

#include <tlQtWidget/Spacer.h>
#include <tlQtWidget/TimeLabel.h>
#include <tlQtWidget/TimeSpinBox.h>
#include <tlQtWidget/TimelineViewport.h>
#include <tlQtWidget/TimelineWidget.h>
#include <tlQtWidget/Util.h>

#include <tlQt/OutputDevice.h>

#include <tlPlay/FilesModel.h>

#include <tlCore/File.h>
#include <tlCore/String.h>
#include <tlCore/StringFormat.h>

#include <QComboBox>
#include <QDoubleSpinBox>
#include <QDragEnterEvent>
#include <QDragMoveEvent>
#include <QHBoxLayout>
#include <QLabel>
#include <QMenuBar>
#include <QMimeData>
#include <QMouseEvent>
#include <QPointer>
#include <QSlider>
#include <QStatusBar>
#include <QStyle>
#include <QToolBar>
#include <QToolButton>
#include <QVector>
#include <QWindow>

namespace tl
{
    namespace play_qt
    {
        namespace
        {
            const size_t sliderSteps = 100;
            const size_t errorTimeout = 5000;
            const size_t infoLabelMax = 24;
        }

        struct MainWindow::Private
        {
            App* app = nullptr;

            QVector<QPointer<qt::TimelinePlayer> > timelinePlayers;
            bool floatOnTop = false;
            bool secondaryFloatOnTop = false;
            timeline::ColorConfigOptions colorConfigOptions;
            timeline::LUTOptions lutOptions;
            timeline::ImageOptions imageOptions;
            timeline::DisplayOptions displayOptions;
            timeline::CompareOptions compareOptions;
            imaging::VideoLevels outputVideoLevels;
            float volume = 1.F;
            bool mute = false;

            FileActions* fileActions = nullptr;
            CompareActions* compareActions = nullptr;
            WindowActions* windowActions = nullptr;
            ViewActions* viewActions = nullptr;
            RenderActions* renderActions = nullptr;
            PlaybackActions* playbackActions = nullptr;
            AudioActions* audioActions = nullptr;
            ToolActions* toolActions = nullptr;

            qtwidget::TimelineViewport* timelineViewport = nullptr;
            qtwidget::TimelineWidget* timelineWidget = nullptr;
            qtwidget::TimeSpinBox* currentTimeSpinBox = nullptr;
            QDoubleSpinBox* speedSpinBox = nullptr;
            QToolButton* speedButton = nullptr;
            qtwidget::TimeLabel* durationLabel = nullptr;
            QComboBox* timeUnitsComboBox = nullptr;
            QSlider* volumeSlider = nullptr;
            FilesTool* filesTool = nullptr;
            CompareTool* compareTool = nullptr;
            ColorTool* colorTool = nullptr;
            InfoTool* infoTool = nullptr;
            AudioTool* audioTool = nullptr;
            DevicesTool* devicesTool = nullptr;
            SettingsTool* settingsTool = nullptr;
            MessagesTool* messagesTool = nullptr;
            SystemLogTool* systemLogTool = nullptr;
            QLabel* infoLabel = nullptr;
            QLabel* cacheLabel = nullptr;
            QStatusBar* statusBar = nullptr;
            SecondaryWindow* secondaryWindow = nullptr;

            std::shared_ptr<observer::ListObserver<std::shared_ptr<play::FilesModelItem> > > filesObserver;
            std::shared_ptr<observer::ValueObserver<int> > aIndexObserver;
            std::shared_ptr<observer::ListObserver<int> > bIndexesObserver;
            std::shared_ptr<observer::ValueObserver<timeline::CompareOptions> > compareOptionsObserver;
            std::shared_ptr<observer::ValueObserver<timeline::ColorConfigOptions> > colorConfigOptionsObserver;
            std::shared_ptr<observer::ValueObserver<DevicesModelData> > devicesModelObserver;
            std::shared_ptr<observer::ListObserver<log::Item> > logObserver;

            bool mousePressed = false;
            math::Vector2i mousePos;
        };

        MainWindow::MainWindow(App* app, QWidget* parent) :
            QMainWindow(parent),
            _p(new Private)
        {
            TLRENDER_P();

            p.app = app;
            p.lutOptions = app->lutOptions();
            p.imageOptions = app->imageOptions();
            p.displayOptions = app->displayOptions();
            p.volume = app->volume();
            p.mute = app->isMuted();

            setFocusPolicy(Qt::StrongFocus);
            setAcceptDrops(true);

            p.fileActions = new FileActions(app, this);
            p.compareActions = new CompareActions(app, this);
            p.windowActions = new WindowActions(app, this);
            p.viewActions = new ViewActions(app, this);
            p.renderActions = new RenderActions(app, this);
            p.playbackActions = new PlaybackActions(app, this);
            p.audioActions = new AudioActions(app, this);
            p.toolActions = new ToolActions(app, this);

            auto menuBar = new QMenuBar;
            menuBar->addMenu(p.fileActions->menu());
            menuBar->addMenu(p.compareActions->menu());
            menuBar->addMenu(p.windowActions->menu());
            menuBar->addMenu(p.viewActions->menu());
            menuBar->addMenu(p.renderActions->menu());
            menuBar->addMenu(p.playbackActions->menu());
            menuBar->addMenu(p.audioActions->menu());
            menuBar->addMenu(p.toolActions->menu());
            setMenuBar(menuBar);

            auto fileToolBar = new QToolBar;
            fileToolBar->setObjectName("FileToolBar");
            fileToolBar->setWindowTitle("File Tool Bar");
            fileToolBar->setIconSize(QSize(20, 20));
            fileToolBar->setAllowedAreas(Qt::TopToolBarArea);
            fileToolBar->setFloatable(false);
            fileToolBar->addAction(p.fileActions->actions()["Open"]);
            fileToolBar->addAction(p.fileActions->actions()["OpenSeparateAudio"]);
            fileToolBar->addAction(p.fileActions->actions()["Close"]);
            fileToolBar->addAction(p.fileActions->actions()["CloseAll"]);
            addToolBar(Qt::TopToolBarArea, fileToolBar);

            auto compareToolBar = new QToolBar;
            compareToolBar->setObjectName("CompareToolBar");
            compareToolBar->setWindowTitle("Compare Tool Bar");
            compareToolBar->setIconSize(QSize(20, 20));
            compareToolBar->setAllowedAreas(Qt::TopToolBarArea);
            compareToolBar->setFloatable(false);
            compareToolBar->addAction(p.compareActions->actions()["A"]);
            compareToolBar->addAction(p.compareActions->actions()["B"]);
            compareToolBar->addAction(p.compareActions->actions()["Wipe"]);
            compareToolBar->addAction(p.compareActions->actions()["Overlay"]);
            compareToolBar->addAction(p.compareActions->actions()["Difference"]);
            compareToolBar->addAction(p.compareActions->actions()["Horizontal"]);
            compareToolBar->addAction(p.compareActions->actions()["Vertical"]);
            compareToolBar->addAction(p.compareActions->actions()["Tile"]);
            addToolBar(Qt::TopToolBarArea, compareToolBar);

            auto windowToolBar = new QToolBar;
            windowToolBar->setObjectName("WindowToolBar");
            windowToolBar->setWindowTitle("Window Tool Bar");
            windowToolBar->setIconSize(QSize(20, 20));
            windowToolBar->setAllowedAreas(Qt::TopToolBarArea);
            windowToolBar->setFloatable(false);
            windowToolBar->addAction(p.windowActions->actions()["FullScreen"]);
            windowToolBar->addAction(p.windowActions->actions()["Secondary"]);
            addToolBar(Qt::TopToolBarArea, windowToolBar);

            auto viewToolBar = new QToolBar;
            viewToolBar->setObjectName("ViewToolBar");
            viewToolBar->setWindowTitle("View Tool Bar");
            viewToolBar->setIconSize(QSize(20, 20));
            viewToolBar->setAllowedAreas(Qt::TopToolBarArea);
            viewToolBar->setFloatable(false);
            viewToolBar->addAction(p.viewActions->actions()["Frame"]);
            viewToolBar->addAction(p.viewActions->actions()["Zoom1To1"]);
            addToolBar(Qt::TopToolBarArea, viewToolBar);

            auto toolsToolBar = new QToolBar;
            toolsToolBar->setObjectName("ToolsToolBar");
            toolsToolBar->setWindowTitle("Tools Tool Bar");
            toolsToolBar->setIconSize(QSize(20, 20));
            toolsToolBar->setAllowedAreas(Qt::TopToolBarArea);
            toolsToolBar->setFloatable(false);
            addToolBar(Qt::TopToolBarArea, toolsToolBar);

            auto context = app->getContext();
            p.timelineViewport = new qtwidget::TimelineViewport(context);
            p.timelineViewport->installEventFilter(this);
            setCentralWidget(p.timelineViewport);

            p.timelineWidget = new qtwidget::TimelineWidget(
                ui::Style::create(context),
                app->timeUnitsModel(),
                context);
            p.timelineWidget->setScrollBarsVisible(false);
            auto timelineDockWidget = new QDockWidget;
            timelineDockWidget->setObjectName("Timeline");
            timelineDockWidget->setWindowTitle(tr("Timeline"));
            timelineDockWidget->setFeatures(QDockWidget::DockWidgetClosable);
            timelineDockWidget->setTitleBarWidget(new QWidget);
            timelineDockWidget->setWidget(p.timelineWidget);
            addDockWidget(Qt::BottomDockWidgetArea, timelineDockWidget);

            p.currentTimeSpinBox = new qtwidget::TimeSpinBox;
            p.currentTimeSpinBox->setTimeObject(app->timeObject());
            p.currentTimeSpinBox->setToolTip(tr("Current time"));
            p.speedSpinBox = new QDoubleSpinBox;
            p.speedSpinBox->setRange(0.0, 1000000.0);
            p.speedSpinBox->setSingleStep(1.0);
            const QFont fixedFont("Noto Mono");
            p.speedSpinBox->setFont(fixedFont);
            p.speedSpinBox->setToolTip(tr("Timeline speed (frames per second)"));
            p.speedButton = new QToolButton;
            p.speedButton->setText(tr("FPS"));
            p.speedButton->setPopupMode(QToolButton::InstantPopup);
            p.speedButton->setMenu(p.playbackActions->speedMenu());
            p.speedButton->setToolTip(tr("Playback speed"));
            p.durationLabel = new qtwidget::TimeLabel;
            p.durationLabel->setTimeObject(app->timeObject());
            p.durationLabel->setFont(fixedFont);
            p.durationLabel->setToolTip(tr("Timeline duration"));
            p.durationLabel->setContentsMargins(5, 0, 5, 0);
            p.timeUnitsComboBox = new QComboBox;
            for (const auto& label : timeline::getTimeUnitsLabels())
            {
                p.timeUnitsComboBox->addItem(QString::fromUtf8(label.c_str()));
            }
            p.timeUnitsComboBox->setCurrentIndex(static_cast<int>(app->timeObject()->timeUnits()));
            p.timeUnitsComboBox->setToolTip(tr("Time units"));
            p.volumeSlider = new QSlider(Qt::Horizontal);
            p.volumeSlider->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);
            p.volumeSlider->setToolTip(tr("Audio volume"));
            auto bottomToolBar = new QToolBar;
            bottomToolBar->setObjectName("BottomToolBar");
            bottomToolBar->setWindowTitle(tr("Bottom Tool Bar"));
            bottomToolBar->setIconSize(QSize(20, 20));
            bottomToolBar->setAllowedAreas(Qt::BottomToolBarArea);
            bottomToolBar->setFloatable(false);
            bottomToolBar->addAction(p.playbackActions->actions()["Reverse"]);
            bottomToolBar->addAction(p.playbackActions->actions()["Stop"]);
            bottomToolBar->addAction(p.playbackActions->actions()["Forward"]);
            bottomToolBar->addAction(p.playbackActions->actions()["Start"]);
            bottomToolBar->addAction(p.playbackActions->actions()["FramePrev"]);
            bottomToolBar->addAction(p.playbackActions->actions()["FrameNext"]);
            bottomToolBar->addAction(p.playbackActions->actions()["End"]);
            bottomToolBar->addWidget(p.currentTimeSpinBox);
            bottomToolBar->addWidget(p.speedSpinBox);
            bottomToolBar->addWidget(p.speedButton);
            bottomToolBar->addWidget(p.durationLabel);
            bottomToolBar->addWidget(p.timeUnitsComboBox);
            bottomToolBar->addWidget(new qtwidget::Spacer(Qt::Horizontal));
            bottomToolBar->addAction(p.audioActions->actions()["Mute"]);
            bottomToolBar->addWidget(p.volumeSlider);
            addToolBar(Qt::BottomToolBarArea, bottomToolBar);

            p.windowActions->menu()->addSeparator();
            p.windowActions->menu()->addAction(fileToolBar->toggleViewAction());
            p.windowActions->menu()->addAction(compareToolBar->toggleViewAction());
            p.windowActions->menu()->addAction(windowToolBar->toggleViewAction());
            p.windowActions->menu()->addAction(viewToolBar->toggleViewAction());
            p.windowActions->menu()->addAction(toolsToolBar->toggleViewAction());
            p.windowActions->menu()->addAction(timelineDockWidget->toggleViewAction());
            p.windowActions->menu()->addAction(bottomToolBar->toggleViewAction());

            p.filesTool = new FilesTool(p.fileActions->actions(), app);
            auto filesDockWidget = new FilesDockWidget(p.filesTool);
            filesDockWidget->hide();
            p.toolActions->menu()->addAction(filesDockWidget->toggleViewAction());
            toolsToolBar->addAction(filesDockWidget->toggleViewAction());
            addDockWidget(Qt::RightDockWidgetArea, filesDockWidget);

            p.compareTool = new CompareTool(p.compareActions->actions(), app);
            auto compareDockWidget = new CompareDockWidget(p.compareTool);
            compareDockWidget->hide();
            p.toolActions->menu()->addAction(compareDockWidget->toggleViewAction());
            toolsToolBar->addAction(compareDockWidget->toggleViewAction());
            addDockWidget(Qt::RightDockWidgetArea, compareDockWidget);

            p.colorTool = new ColorTool(app);
            auto colorDockWidget = new ColorDockWidget(p.colorTool);
            colorDockWidget->hide();
            p.toolActions->menu()->addAction(colorDockWidget->toggleViewAction());
            toolsToolBar->addAction(colorDockWidget->toggleViewAction());
            addDockWidget(Qt::RightDockWidgetArea, colorDockWidget);

            p.infoTool = new InfoTool(app);
            auto infoDockWidget = new InfoDockWidget(p.infoTool);
            infoDockWidget->hide();
            p.toolActions->menu()->addAction(infoDockWidget->toggleViewAction());
            toolsToolBar->addAction(infoDockWidget->toggleViewAction());
            addDockWidget(Qt::RightDockWidgetArea, infoDockWidget);

            p.audioTool = new AudioTool(app);
            auto audioDockWidget = new AudioDockWidget(p.audioTool);
            audioDockWidget->hide();
            p.toolActions->menu()->addAction(audioDockWidget->toggleViewAction());
            toolsToolBar->addAction(audioDockWidget->toggleViewAction());
            addDockWidget(Qt::RightDockWidgetArea, audioDockWidget);

            p.devicesTool = new DevicesTool(app);
            auto deviceDockWidget = new DevicesDockWidget(p.devicesTool);
            deviceDockWidget->hide();
            p.toolActions->menu()->addAction(deviceDockWidget->toggleViewAction());
            toolsToolBar->addAction(deviceDockWidget->toggleViewAction());
            addDockWidget(Qt::RightDockWidgetArea, deviceDockWidget);

            p.settingsTool = new SettingsTool(app);
            auto settingsDockWidget = new SettingsDockWidget(p.settingsTool);
            settingsDockWidget->hide();
            p.toolActions->menu()->addAction(settingsDockWidget->toggleViewAction());
            toolsToolBar->addAction(settingsDockWidget->toggleViewAction());
            addDockWidget(Qt::RightDockWidgetArea, settingsDockWidget);

            p.messagesTool = new MessagesTool(app);
            auto messagesDockWidget = new MessagesDockWidget(p.messagesTool);
            messagesDockWidget->hide();
            p.toolActions->menu()->addAction(messagesDockWidget->toggleViewAction());
            toolsToolBar->addAction(messagesDockWidget->toggleViewAction());
            addDockWidget(Qt::RightDockWidgetArea, messagesDockWidget);

            p.systemLogTool = new SystemLogTool(app);
            auto systemLogDockWidget = new SystemLogDockWidget(p.systemLogTool);
            systemLogDockWidget->hide();
            systemLogDockWidget->toggleViewAction()->setShortcut(QKeySequence(Qt::Key_F11));
            p.toolActions->menu()->addAction(systemLogDockWidget->toggleViewAction());
            addDockWidget(Qt::RightDockWidgetArea, systemLogDockWidget);

            p.infoLabel = new QLabel;
            p.cacheLabel = new QLabel;

            p.statusBar = new QStatusBar;
            auto hLayout = new QHBoxLayout;
            hLayout->addWidget(p.infoLabel);
            hLayout->addWidget(p.cacheLabel);
            auto labelWidget = new QWidget;
            labelWidget->setLayout(hLayout);
            p.statusBar->addPermanentWidget(labelWidget);
            setStatusBar(p.statusBar);

            p.timelineViewport->setFocus();

            _timelinePlayersUpdate();
            _widgetUpdate();

            p.filesObserver = observer::ListObserver<std::shared_ptr<play::FilesModelItem> >::create(
                app->filesModel()->observeFiles(),
                [this](const std::vector<std::shared_ptr<play::FilesModelItem> >&)
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

            p.colorConfigOptionsObserver = observer::ValueObserver<timeline::ColorConfigOptions>::create(
                app->colorModel()->observeConfigOptions(),
                [this](const timeline::ColorConfigOptions& value)
                {
                    _p->colorConfigOptions = value;
                    _widgetUpdate();
                });

            p.devicesModelObserver = observer::ValueObserver<DevicesModelData>::create(
                app->devicesModel()->observeData(),
                [this](const DevicesModelData& value)
                {
                    _p->outputVideoLevels = value.videoLevels;
                    _widgetUpdate();
                });

            p.logObserver = observer::ListObserver<log::Item>::create(
                context->getLogSystem()->observeLog(),
                [this](const std::vector<log::Item>& value)
                {
                    for (const auto& i : value)
                    {
                        switch (i.type)
                        {
                        case log::Type::Error:
                            _p->statusBar->showMessage(
                                QString::fromUtf8(log::toString(i).c_str()),
                                errorTimeout);
                            break;
                        default: break;
                        }
                    }
                });

            connect(
                app,
                &App::lutOptionsChanged,
                [this](const timeline::LUTOptions& value)
                {
                    _p->lutOptions = value;
                    _widgetUpdate();
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
                app,
                &App::displayOptionsChanged,
                [this](const timeline::DisplayOptions& value)
                {
                    _p->displayOptions = value;
                    _widgetUpdate();
                });
            connect(
                app,
                &App::volumeChanged,
                [this](float value)
                {
                    _p->volume = value;
                    _widgetUpdate();
                });
            connect(
                app,
                &App::muteChanged,
                [this](bool value)
                {
                    _p->mute = value;
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
                &QAction::toggled,
                [this](bool value)
                {
                    _p->timelineViewport->setFrameView(value);
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
                p.timelineWidget,
                &qtwidget::TimelineWidget::frameViewChanged,
                [this](bool value)
                {
                    _p->playbackActions->actions()["Timeline/FrameView"]->setChecked(value);
                });

            connect(
                p.currentTimeSpinBox,
                &qtwidget::TimeSpinBox::valueChanged,
                [this](const otime::RationalTime& value)
                {
                    if (!_p->timelinePlayers.isEmpty())
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
                p.timeUnitsComboBox,
                QOverload<int>::of(&QComboBox::currentIndexChanged),
                [app](int value)
                {
                    app->timeObject()->setTimeUnits(
                        static_cast<timeline::TimeUnits>(value));
                });

            connect(
                p.volumeSlider,
                SIGNAL(valueChanged(int)),
                SLOT(_volumeCallback(int)));

            connect(
                p.colorTool,
                &ColorTool::lutOptionsChanged,
                [app](const timeline::LUTOptions& value)
                {
                    app->setLUTOptions(value);
                });
            connect(
                p.colorTool,
                &ColorTool::displayOptionsChanged,
                [app](const timeline::DisplayOptions& value)
                {
                    app->setDisplayOptions(value);
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
                p.timelineViewport,
                &qtwidget::TimelineViewport::viewPosAndZoomChanged,
                [this](const tl::math::Vector2i& pos, float zoom)
                {
                    _p->app->outputDevice()->setView(
                        pos,
                        zoom,
                        _p->timelineViewport->hasFrameView());
                });
            connect(
                p.timelineViewport,
                &qtwidget::TimelineViewport::frameViewChanged,
                [this](bool value)
                {
                    _p->viewActions->actions()["Frame"]->setChecked(value);

                    _p->app->outputDevice()->setView(
                        _p->timelineViewport->viewPos(),
                        _p->timelineViewport->viewZoom(),
                        value);
                });

            connect(
                app->timeObject(),
                &qt::TimeObject::timeUnitsChanged,
                [this](timeline::TimeUnits value)
                {
                    _p->timeUnitsComboBox->setCurrentIndex(
                        static_cast<int>(value));
                });

            connect(
                app->settingsObject(),
                &SettingsObject::valueChanged,
                [this](const QString& name, const QVariant& value)
                {
                    if ("Timeline/FrameView" == name)
                    {
                        _p->timelineWidget->setFrameView(value.toBool());
                    }
                    else if ("Timeline/StopOnScrub" == name)
                    {
                        _p->timelineWidget->setStopOnScrub(value.toBool());
                    }
                    else if ("Timeline/Thumbnails" == name)
                    {
                        auto itemOptions = _p->timelineWidget->itemOptions();
                        itemOptions.thumbnails = value.toBool();
                        _p->timelineWidget->setItemOptions(itemOptions);
                    }
                    else if ("Timeline/ThumbnailsSize" == name)
                    {
                        auto itemOptions = _p->timelineWidget->itemOptions();
                        itemOptions.thumbnailHeight = value.toInt();
                        itemOptions.waveformHeight = itemOptions.thumbnailHeight / 2;
                        _p->timelineWidget->setItemOptions(itemOptions);
                    }
                    else if ("Timeline/Transitions" == name)
                    {
                        auto itemOptions = _p->timelineWidget->itemOptions();
                        itemOptions.showTransitions = value.toBool();
                        _p->timelineWidget->setItemOptions(itemOptions);
                    }
                    else if ("Timeline/Markers" == name)
                    {
                        auto itemOptions = _p->timelineWidget->itemOptions();
                        itemOptions.showMarkers = value.toBool();
                        _p->timelineWidget->setItemOptions(itemOptions);
                    }
                });

            auto settingsObject = app->settingsObject();
            settingsObject->setDefaultValue("MainWindow/geometry", QByteArray());
            auto ba = settingsObject->value("MainWindow/geometry").toByteArray();
            if (!ba.isEmpty())
            {
                restoreGeometry(ba);
            }
            else
            {
                resize(1280, 720);
            }
            settingsObject->setDefaultValue("MainWindow/windowState", QByteArray());
            ba = settingsObject->value("MainWindow/windowState").toByteArray();
            if (!ba.isEmpty())
            {
                restoreState(ba);
            }
            settingsObject->setDefaultValue("MainWindow/FloatOnTop", false);
            p.floatOnTop = settingsObject->value("MainWindow/FloatOnTop").toBool();
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
            settingsObject->setDefaultValue("MainWindow/SecondaryFloatOnTop", false);
            p.secondaryFloatOnTop = settingsObject->value("MainWindow/SecondaryFloatOnTop").toBool();
            {
                QSignalBlocker blocker(p.windowActions->actions()["SecondaryFloatOnTop"]);
                p.windowActions->actions()["SecondaryFloatOnTop"]->setChecked(p.secondaryFloatOnTop);
            }
        }

        MainWindow::~MainWindow()
        {
            TLRENDER_P();
            auto settingsObject = p.app->settingsObject();
            settingsObject->setValue("MainWindow/geometry", saveGeometry());
            settingsObject->setValue("MainWindow/windowState", saveState());
            settingsObject->setValue("MainWindow/FloatOnTop", p.floatOnTop);
            settingsObject->setValue("MainWindow/SecondaryFloatOnTop", p.secondaryFloatOnTop);
            if (p.secondaryWindow)
            {
                delete p.secondaryWindow;
                p.secondaryWindow = nullptr;
            }
        }

        void MainWindow::setTimelinePlayers(const QVector<QPointer<qt::TimelinePlayer> >& timelinePlayers)
        {
            TLRENDER_P();
            if (!p.timelinePlayers.empty() && p.timelinePlayers[0])
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
                    SIGNAL(audioOffsetChanged(double)),
                    p.audioTool,
                    SLOT(setAudioOffset(double)));
            }

            p.timelinePlayers = timelinePlayers;

            if (!p.timelinePlayers.empty() && p.timelinePlayers[0])
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
                    SIGNAL(audioOffsetChanged(double)),
                    p.audioTool,
                    SLOT(setAudioOffset(double)));
            }

            _timelinePlayersUpdate();
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
                    const float devicePixelRatio = p.timelineViewport->window()->devicePixelRatio();
                    p.mousePos.x = mouseEvent->x() * devicePixelRatio;
                    p.mousePos.y = p.timelineViewport->height() * devicePixelRatio - 1 -
                        mouseEvent->y() * devicePixelRatio;
                    if (p.mousePressed)
                    {
                        if (!p.timelinePlayers.empty() && p.timelinePlayers[0])
                        {
                            const auto& ioInfo = p.timelinePlayers[0]->ioInfo();
                            if (!ioInfo.video.empty())
                            {
                                const auto& imageInfo = ioInfo.video[0];
                                const math::Vector2i& viewPos = p.timelineViewport->viewPos();
                                const float viewZoom = p.timelineViewport->viewZoom();
                                auto compareOptions = p.compareOptions;
                                compareOptions.wipeCenter.x = (p.mousePos.x - viewPos.x) / viewZoom /
                                    static_cast<float>(imageInfo.size.w * imageInfo.size.pixelAspectRatio);
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
                p.secondaryWindow->viewport()->setColorConfigOptions(p.colorConfigOptions);
                p.secondaryWindow->viewport()->setLUTOptions(p.lutOptions);
                std::vector<timeline::ImageOptions> imageOptions;
                std::vector<timeline::DisplayOptions> displayOptions;
                for (const auto& i : p.timelinePlayers)
                {
                    imageOptions.push_back(p.imageOptions);
                    displayOptions.push_back(p.displayOptions);
                }
                p.secondaryWindow->viewport()->setImageOptions(imageOptions);
                p.secondaryWindow->viewport()->setDisplayOptions(displayOptions);
                p.secondaryWindow->viewport()->setCompareOptions(p.compareOptions);
                p.secondaryWindow->viewport()->setTimelinePlayers(p.timelinePlayers);

                connect(
                    p.timelineViewport,
                    SIGNAL(viewPosAndZoomChanged(const tl::math::Vector2i&, float)),
                    p.secondaryWindow->viewport(),
                    SLOT(setViewPosAndZoom(const tl::math::Vector2i&, float)));
                connect(
                    p.timelineViewport,
                    SIGNAL(frameViewChanged(bool)),
                    p.secondaryWindow->viewport(),
                    SLOT(setFrameView(bool)));

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
            {
                QSignalBlocker blocker(p.windowActions->actions()["Secondary"]);
                p.windowActions->actions()["Secondary"]->setChecked(false);
            }
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
            p.app->setVolume(value / static_cast<float>(sliderSteps));
        }

        void MainWindow::_timelinePlayersUpdate()
        {
            TLRENDER_P();
            p.playbackActions->setTimelinePlayers(p.timelinePlayers);
            p.audioActions->setTimelinePlayers(p.timelinePlayers);
            p.timelineViewport->setTimelinePlayers(p.timelinePlayers);
            if (p.secondaryWindow)
            {
                p.secondaryWindow->viewport()->setTimelinePlayers(p.timelinePlayers);
            }
            p.app->outputDevice()->setTimelinePlayers(p.timelinePlayers);
        }

        void MainWindow::_widgetUpdate()
        {
            TLRENDER_P();

            const auto& files = p.app->filesModel()->observeFiles()->get();
            const size_t count = files.size();
            p.timelineWidget->setEnabled(count > 0);
            p.currentTimeSpinBox->setEnabled(count > 0);
            p.speedSpinBox->setEnabled(count > 0);
            p.volumeSlider->setEnabled(count > 0);

            if (!p.timelinePlayers.empty() && p.timelinePlayers[0])
            {
                {
                    QSignalBlocker blocker(p.currentTimeSpinBox);
                    p.currentTimeSpinBox->setValue(p.timelinePlayers[0]->currentTime());
                }

                {
                    QSignalBlocker blocker(p.speedSpinBox);
                    p.speedSpinBox->setValue(p.timelinePlayers[0]->speed());
                }

                const auto& timeRange = p.timelinePlayers[0]->timeRange();
                p.durationLabel->setValue(timeRange.duration());

                {
                    QSignalBlocker blocker(p.volumeSlider);
                    p.volumeSlider->setValue(p.timelinePlayers[0]->volume() * sliderSteps);
                }
            }
            else
            {
                {
                    QSignalBlocker blocker(p.currentTimeSpinBox);
                    p.currentTimeSpinBox->setValue(time::invalidTime);
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

            p.viewActions->actions()["Frame"]->setChecked(p.timelineViewport->hasFrameView());

            p.renderActions->setImageOptions(p.imageOptions);
            p.renderActions->setDisplayOptions(p.displayOptions);

            p.timelineViewport->setColorConfigOptions(p.colorConfigOptions);
            p.timelineViewport->setLUTOptions(p.lutOptions);
            std::vector<timeline::ImageOptions> imageOptions;
            std::vector<timeline::DisplayOptions> displayOptions;
            for (const auto& i : p.timelinePlayers)
            {
                imageOptions.push_back(p.imageOptions);
                displayOptions.push_back(p.displayOptions);
            }
            p.timelineViewport->setImageOptions(imageOptions);
            p.timelineViewport->setDisplayOptions(displayOptions);
            p.timelineViewport->setCompareOptions(p.compareOptions);

            p.timelineWidget->setPlayer(
                (!p.timelinePlayers.empty() && p.timelinePlayers[0]) ?
                p.timelinePlayers[0]->player() :
                nullptr);
            auto settingsObject = p.app->settingsObject();
            p.timelineWidget->setFrameView(settingsObject->value("Timeline/FrameView").toBool());
            p.timelineWidget->setStopOnScrub(settingsObject->value("Timeline/StopOnScrub").toBool());
            auto itemOptions = p.timelineWidget->itemOptions();
            itemOptions.thumbnails = settingsObject->value("Timeline/Thumbnails").toBool();
            itemOptions.thumbnailHeight = settingsObject->value("Timeline/ThumbnailsSize").toInt();
            itemOptions.waveformHeight = itemOptions.thumbnailHeight / 2;
            itemOptions.showTransitions = settingsObject->value("Timeline/Transitions").toBool();
            itemOptions.showMarkers = settingsObject->value("Timeline/Markers").toBool();
            p.timelineWidget->setItemOptions(itemOptions);

            {
                const QSignalBlocker blocker(p.volumeSlider);
                p.volumeSlider->setValue(p.volume * sliderSteps);
            }

            p.colorTool->setLUTOptions(p.lutOptions);
            p.colorTool->setDisplayOptions(p.displayOptions);

            p.infoTool->setInfo(
                (!p.timelinePlayers.empty() && p.timelinePlayers[0]) ?
                p.timelinePlayers[0]->ioInfo() :
                io::Info());

            p.audioTool->setAudioOffset(
                (!p.timelinePlayers.empty() && p.timelinePlayers[0])
                ? p.timelinePlayers[0]->audioOffset() :
                0.0);

            std::vector<std::string> infoLabel;
            std::vector<std::string> infoTooltip;
            if (!p.timelinePlayers.empty() && p.timelinePlayers[0])
            {
                const std::string fileName = p.timelinePlayers[0]->path().get(-1, false);
                std::string fileNameLabel = fileName;
                if (fileNameLabel.size() > infoLabelMax)
                {
                    fileNameLabel.replace(infoLabelMax, fileNameLabel.size() - infoLabelMax, "...");
                }
                infoLabel.push_back(fileNameLabel);
                infoTooltip.push_back(fileName);

                const io::Info& ioInfo = p.timelinePlayers[0]->ioInfo();
                if (!ioInfo.video.empty())
                {
                    {
                        std::stringstream ss;
                        ss.precision(2);
                        ss << "V:" <<
                            ioInfo.video[0].size.w << "x" <<
                            ioInfo.video[0].size.h << ":" <<
                            std::fixed << ioInfo.video[0].size.getAspect() << " " <<
                            ioInfo.video[0].pixelType;
                        infoLabel.push_back(ss.str());
                    }
                    {
                        std::stringstream ss;
                        ss.precision(2);
                        ss << "Video :" <<
                            ioInfo.video[0].size.w << "x" <<
                            ioInfo.video[0].size.h << ":" <<
                            std::fixed << ioInfo.video[0].size.getAspect() << " " <<
                            ioInfo.video[0].pixelType;
                        infoTooltip.push_back(ss.str());
                    }
                }
                if (ioInfo.audio.isValid())
                {
                    {
                        std::stringstream ss;
                        ss << "A: " <<
                            static_cast<size_t>(ioInfo.audio.channelCount) << " " <<
                            ioInfo.audio.dataType << " " <<
                            ioInfo.audio.sampleRate;
                        infoLabel.push_back(ss.str());
                    }
                    {
                        std::stringstream ss;
                        ss << "Audio: " <<
                            static_cast<size_t>(ioInfo.audio.channelCount) << " " <<
                            ioInfo.audio.dataType << " " <<
                            ioInfo.audio.sampleRate;
                        infoTooltip.push_back(ss.str());
                    }
                }
            }
            p.infoLabel->setText(QString::fromUtf8(string::join(infoLabel, ", ").c_str()));
            p.infoLabel->setToolTip(QString::fromUtf8(string::join(infoTooltip, "\n").c_str()));

            if (p.secondaryWindow)
            {
                p.secondaryWindow->viewport()->setColorConfigOptions(p.colorConfigOptions);
                p.secondaryWindow->viewport()->setLUTOptions(p.lutOptions);
                p.secondaryWindow->viewport()->setImageOptions(imageOptions);
                p.secondaryWindow->viewport()->setDisplayOptions(displayOptions);
                p.secondaryWindow->viewport()->setCompareOptions(p.compareOptions);
            }

            p.app->outputDevice()->setColorConfigOptions(p.colorConfigOptions);
            p.app->outputDevice()->setLUTOptions(p.lutOptions);
            p.app->outputDevice()->setImageOptions(imageOptions);
            for (auto& i : displayOptions)
            {
                i.videoLevels = p.outputVideoLevels;
            }
            p.app->outputDevice()->setDisplayOptions(displayOptions);
            p.app->outputDevice()->setCompareOptions(p.compareOptions);
        }
    }
}
