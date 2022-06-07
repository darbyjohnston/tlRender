// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2022 Darby Johnston
// All rights reserved.

#include <tlPlayApp/MainWindow.h>

#include <tlPlayApp/App.h>
#include <tlPlayApp/AudioActions.h>
#include <tlPlayApp/AudioTool.h>
#include <tlPlayApp/ColorModel.h>
#include <tlPlayApp/ColorTool.h>
#include <tlPlayApp/CompareActions.h>
#include <tlPlayApp/CompareTool.h>
#include <tlPlayApp/DevicesModel.h>
#include <tlPlayApp/DevicesTool.h>
#include <tlPlayApp/FileActions.h>
#include <tlPlayApp/FilesModel.h>
#include <tlPlayApp/FilesTool.h>
#include <tlPlayApp/ImageActions.h>
#include <tlPlayApp/InfoTool.h>
#include <tlPlayApp/MessagesTool.h>
#include <tlPlayApp/PlaybackActions.h>
#include <tlPlayApp/SecondaryWindow.h>
#include <tlPlayApp/SettingsObject.h>
#include <tlPlayApp/SettingsTool.h>
#include <tlPlayApp/SystemLogTool.h>
#include <tlPlayApp/ViewActions.h>
#include <tlPlayApp/WindowActions.h>

#include <tlQtWidget/Spacer.h>
#include <tlQtWidget/TimeLabel.h>
#include <tlQtWidget/TimeSpinBox.h>
#include <tlQtWidget/TimelineSlider.h>
#include <tlQtWidget/TimelineViewport.h>
#include <tlQtWidget/Util.h>

#include <tlQt/OutputDevice.h>

#include <tlCore/File.h>
#include <tlCore/String.h>
#include <tlCore/StringFormat.h>

#include <QDoubleSpinBox>
#include <QDragEnterEvent>
#include <QDragMoveEvent>
#include <QHBoxLayout>
#include <QLabel>
#include <QMenuBar>
#include <QMimeData>
#include <QMouseEvent>
#include <QSlider>
#include <QStatusBar>
#include <QStyle>
#include <QToolBar>
#include <QToolButton>
#include <QWindow>

namespace tl
{
    namespace play
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

            std::vector<qt::TimelinePlayer*> timelinePlayers;
            bool floatOnTop = false;
            bool secondaryFloatOnTop = false;
            imaging::ColorConfig colorConfig;
            timeline::ImageOptions imageOptions;
            timeline::DisplayOptions displayOptions;
            timeline::CompareOptions compareOptions;

            FileActions* fileActions = nullptr;
            CompareActions* compareActions = nullptr;
            ViewActions* viewActions = nullptr;
            ImageActions* imageActions = nullptr;
            PlaybackActions* playbackActions = nullptr;
            AudioActions* audioActions = nullptr;
            WindowActions* windowActions = nullptr;

            qtwidget::TimelineViewport* timelineViewport = nullptr;
            qtwidget::TimelineSlider* timelineSlider = nullptr;
            qtwidget::TimeSpinBox* currentTimeSpinBox = nullptr;
            qtwidget::TimeLabel* durationLabel = nullptr;
            QToolButton* timeUnitsButton = nullptr;
            QDoubleSpinBox* speedSpinBox = nullptr;
            QToolButton* speedButton = nullptr;
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
            QStatusBar* statusBar = nullptr;
            SecondaryWindow* secondaryWindow = nullptr;
            qt::OutputDevice* outputDevice = nullptr;

            std::shared_ptr<observer::ListObserver<std::shared_ptr<FilesModelItem> > > filesObserver;
            std::shared_ptr<observer::ValueObserver<int> > aIndexObserver;
            std::shared_ptr<observer::ListObserver<int> > bIndexesObserver;
            std::shared_ptr<observer::ListObserver<timeline::ImageOptions> > imageOptionsObserver;
            std::shared_ptr<observer::ListObserver<timeline::DisplayOptions> > displayOptionsObserver;
            std::shared_ptr<observer::ValueObserver<timeline::CompareOptions> > compareOptionsObserver;
            std::shared_ptr<observer::ValueObserver<imaging::ColorConfig> > colorConfigObserver;
            std::shared_ptr<observer::ValueObserver<DevicesModelData> > devicesObserver;
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
            p.imageOptions = app->imageOptions();
            p.displayOptions = app->displayOptions();

            setFocusPolicy(Qt::ClickFocus);
            setAcceptDrops(true);

            p.fileActions = new FileActions(app, this);
            p.compareActions = new CompareActions(app, this);
            p.viewActions = new ViewActions(app, this);
            p.imageActions = new ImageActions(app, this);
            p.playbackActions = new PlaybackActions(app, this);
            p.audioActions = new AudioActions(app, this);
            p.windowActions = new WindowActions(app, this);

            auto menuBar = new QMenuBar;
            menuBar->addMenu(p.fileActions->menu());
            menuBar->addMenu(p.compareActions->menu());
            menuBar->addMenu(p.viewActions->menu());
            menuBar->addMenu(p.imageActions->menu());
            menuBar->addMenu(p.playbackActions->menu());
            menuBar->addMenu(p.audioActions->menu());
            menuBar->addMenu(p.windowActions->menu());
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

            auto viewToolBar = new QToolBar;
            viewToolBar->setObjectName("ViewToolBar");
            viewToolBar->setWindowTitle("View Tool Bar");
            viewToolBar->setIconSize(QSize(20, 20));
            viewToolBar->setAllowedAreas(Qt::TopToolBarArea);
            viewToolBar->setFloatable(false);
            viewToolBar->addAction(p.viewActions->actions()["Frame"]);
            viewToolBar->addAction(p.viewActions->actions()["Zoom1To1"]);
            addToolBar(Qt::TopToolBarArea, viewToolBar);

            auto windowToolBar = new QToolBar;
            windowToolBar->setObjectName("WindowToolBar");
            windowToolBar->setWindowTitle("Window Tool Bar");
            windowToolBar->setIconSize(QSize(20, 20));
            windowToolBar->setAllowedAreas(Qt::TopToolBarArea);
            windowToolBar->setFloatable(false);
            windowToolBar->addAction(p.windowActions->actions()["FullScreen"]);
            windowToolBar->addAction(p.windowActions->actions()["Secondary"]);
            addToolBar(Qt::TopToolBarArea, windowToolBar);

            p.timelineViewport = new qtwidget::TimelineViewport(app->getContext());
            p.timelineViewport->installEventFilter(this);
            setCentralWidget(p.timelineViewport);

            p.timelineSlider = new qtwidget::TimelineSlider(
                app->thumbnailProvider(),
                app->getContext());
            p.timelineSlider->setTimeObject(app->timeObject());
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
            p.durationLabel->setContentsMargins(5, 0, 5, 0);
            p.timeUnitsButton = new QToolButton;
            p.timeUnitsButton->setText(tr("Time"));
            p.timeUnitsButton->setPopupMode(QToolButton::InstantPopup);
            p.timeUnitsButton->setMenu(p.playbackActions->timeUnitsMenu());
            p.timeUnitsButton->setToolTip(tr("Time units"));
            p.speedSpinBox = new QDoubleSpinBox;
            p.speedSpinBox->setRange(0.0, 120.0);
            p.speedSpinBox->setSingleStep(1.0);
            const QFont fixedFont = qtwidget::font("NotoMono-Regular");
            p.speedSpinBox->setFont(fixedFont);
            p.speedSpinBox->setToolTip(tr("Timeline speed (frames per second)"));
            p.speedButton = new QToolButton;
            p.speedButton->setText(tr("FPS"));
            p.speedButton->setPopupMode(QToolButton::InstantPopup);
            p.speedButton->setMenu(p.playbackActions->speedMenu());
            p.speedButton->setToolTip(tr("Playback speed"));
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
            bottomToolBar->addWidget(p.durationLabel);
            bottomToolBar->addWidget(p.timeUnitsButton);
            bottomToolBar->addWidget(p.speedSpinBox);
            bottomToolBar->addWidget(p.speedButton);
            bottomToolBar->addWidget(new qtwidget::Spacer(Qt::Horizontal));
            bottomToolBar->addAction(p.audioActions->actions()["Mute"]);
            bottomToolBar->addWidget(p.volumeSlider);
            addToolBar(Qt::BottomToolBarArea, bottomToolBar);

            p.windowActions->menu()->addSeparator();
            p.windowActions->menu()->addAction(fileToolBar->toggleViewAction());
            p.windowActions->menu()->addAction(compareToolBar->toggleViewAction());
            p.windowActions->menu()->addAction(windowToolBar->toggleViewAction());
            p.windowActions->menu()->addAction(viewToolBar->toggleViewAction());
            p.windowActions->menu()->addAction(timelineDockWidget->toggleViewAction());
            p.windowActions->menu()->addAction(bottomToolBar->toggleViewAction());

            p.filesTool = new FilesTool(p.fileActions->actions(), app);
            auto filesDockWidget = new FilesDockWidget(p.filesTool);
            filesDockWidget->hide();
            p.windowActions->menu()->addSeparator();
            p.windowActions->menu()->addAction(filesDockWidget->toggleViewAction());
            windowToolBar->addAction(filesDockWidget->toggleViewAction());
            addDockWidget(Qt::RightDockWidgetArea, filesDockWidget);

            p.compareTool = new CompareTool(p.compareActions->actions(), app);
            auto compareDockWidget = new CompareDockWidget(p.compareTool);
            compareDockWidget->hide();
            p.windowActions->menu()->addAction(compareDockWidget->toggleViewAction());
            windowToolBar->addAction(compareDockWidget->toggleViewAction());
            addDockWidget(Qt::RightDockWidgetArea, compareDockWidget);

            p.colorTool = new ColorTool(app->colorModel());
            auto colorDockWidget = new ColorDockWidget(p.colorTool);
            colorDockWidget->hide();
            p.windowActions->menu()->addAction(colorDockWidget->toggleViewAction());
            windowToolBar->addAction(colorDockWidget->toggleViewAction());
            addDockWidget(Qt::RightDockWidgetArea, colorDockWidget);

            p.infoTool = new InfoTool(app);
            auto infoDockWidget = new InfoDockWidget(p.infoTool);
            infoDockWidget->hide();
            p.windowActions->menu()->addAction(infoDockWidget->toggleViewAction());
            windowToolBar->addAction(infoDockWidget->toggleViewAction());
            addDockWidget(Qt::RightDockWidgetArea, infoDockWidget);

            p.audioTool = new AudioTool();
            auto audioDockWidget = new AudioDockWidget(p.audioTool);
            audioDockWidget->hide();
            p.windowActions->menu()->addAction(audioDockWidget->toggleViewAction());
            windowToolBar->addAction(audioDockWidget->toggleViewAction());
            addDockWidget(Qt::RightDockWidgetArea, audioDockWidget);

            p.devicesTool = new DevicesTool(app);
            auto deviceDockWidget = new DevicesDockWidget(p.devicesTool);
            deviceDockWidget->hide();
            p.windowActions->menu()->addAction(deviceDockWidget->toggleViewAction());
            windowToolBar->addAction(deviceDockWidget->toggleViewAction());
            addDockWidget(Qt::RightDockWidgetArea, deviceDockWidget);

            p.settingsTool = new SettingsTool(app->settingsObject(), app->timeObject());
            auto settingsDockWidget = new SettingsDockWidget(p.settingsTool);
            settingsDockWidget->hide();
            p.windowActions->menu()->addAction(settingsDockWidget->toggleViewAction());
            windowToolBar->addAction(settingsDockWidget->toggleViewAction());
            addDockWidget(Qt::RightDockWidgetArea, settingsDockWidget);

            p.messagesTool = new MessagesTool(app->getContext());
            auto messagesDockWidget = new MessagesDockWidget(p.messagesTool);
            messagesDockWidget->hide();
            p.windowActions->menu()->addAction(messagesDockWidget->toggleViewAction());
            windowToolBar->addAction(messagesDockWidget->toggleViewAction());
            addDockWidget(Qt::RightDockWidgetArea, messagesDockWidget);

            p.systemLogTool = new SystemLogTool(app->getContext());
            auto systemLogDockWidget = new SystemLogDockWidget(p.systemLogTool);
            systemLogDockWidget->hide();
            systemLogDockWidget->toggleViewAction()->setShortcut(QKeySequence(Qt::Key_F11));
            p.windowActions->menu()->addAction(systemLogDockWidget->toggleViewAction());
            addDockWidget(Qt::RightDockWidgetArea, systemLogDockWidget);

            p.infoLabel = new QLabel;

            p.statusBar = new QStatusBar;
            p.statusBar->addPermanentWidget(p.infoLabel);
            setStatusBar(p.statusBar);

            p.timelineViewport->setFocus();

            p.outputDevice = new qt::OutputDevice(app->getContext());

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

            p.devicesObserver = observer::ValueObserver<DevicesModelData>::create(
                app->devicesModel()->observeData(),
                [this](const DevicesModelData& value)
                {
                    const device::PixelType pixelType = value.pixelTypeIndex >= 0 &&
                        value.pixelTypeIndex < value.pixelTypes.size() ?
                        value.pixelTypes[value.pixelTypeIndex] :
                        device::PixelType::None;
                    _p->outputDevice->setDevice(
                        value.deviceIndex - 1,
                        value.displayModeIndex - 1,
                        pixelType);
                });

            p.logObserver = observer::ListObserver<log::Item>::create(
                app->getContext()->getLogSystem()->observeLog(),
                [this](const std::vector<log::Item>& value)
                {
                    for (const auto& i : value)
                    {
                        switch (i.type)
                        {
                        case log::Type::Error:
                            _p->statusBar->showMessage(
                                QString(tr("ERROR: %1")).
                                arg(QString::fromUtf8(i.message.c_str())),
                                errorTimeout);
                            break;
                        default: break;
                        }
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
                app,
                &App::displayOptionsChanged,
                [this](const timeline::DisplayOptions& value)
                {
                    _p->displayOptions = value;
                    _widgetUpdate();
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
                    _p->outputDevice->setView(
                        pos,
                        zoom,
                        _p->timelineViewport->hasFrameView());
                });
            connect(
                p.timelineViewport,
                &qtwidget::TimelineViewport::frameViewActivated,
                [this]
                {
                    _p->outputDevice->setView(
                        _p->timelineViewport->viewPos(),
                        _p->timelineViewport->viewZoom(),
                        _p->timelineViewport->hasFrameView());
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
            connect(
                app->settingsObject(),
                &SettingsObject::valueChanged,
                [this](const QString& name, const QVariant& value)
                {
                    if ("Timeline/StopOnScrub" == name)
                    {
                        _p->timelineSlider->setStopOnScrub(value.toBool());
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
            if (p.outputDevice)
            {
                delete p.outputDevice;
                p.outputDevice = nullptr;
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
                    const float devicePixelRatio = p.timelineViewport->window()->devicePixelRatio();
                    p.mousePos.x = mouseEvent->x() * devicePixelRatio;
                    p.mousePos.y = p.timelineViewport->height() * devicePixelRatio - 1 -
                        mouseEvent->y() * devicePixelRatio;
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

            const auto& files = p.app->filesModel()->observeFiles()->get();
            const size_t count = files.size();
            p.timelineSlider->setEnabled(count > 0);
            p.currentTimeSpinBox->setEnabled(count > 0);
            p.speedSpinBox->setEnabled(count > 0);
            p.volumeSlider->setEnabled(count > 0);

            if (!p.timelinePlayers.empty())
            {
                {
                    QSignalBlocker blocker(p.currentTimeSpinBox);
                    p.currentTimeSpinBox->setValue(p.timelinePlayers[0]->currentTime());
                }

                const auto& duration = p.timelinePlayers[0]->duration();
                p.durationLabel->setValue(duration);

                {
                    QSignalBlocker blocker(p.speedSpinBox);
                    p.speedSpinBox->setValue(p.timelinePlayers[0]->speed());
                }

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

                p.durationLabel->setValue(time::invalidTime);

                {
                    QSignalBlocker blocker(p.speedSpinBox);
                    p.speedSpinBox->setValue(0.0);
                }

                {
                    QSignalBlocker blocker(p.volumeSlider);
                    p.volumeSlider->setValue(0);
                }
            }

            p.compareActions->setCompareOptions(p.compareOptions);

            p.imageActions->setImageOptions(p.imageOptions);
            p.imageActions->setDisplayOptions(p.displayOptions);

            p.playbackActions->setTimelinePlayers(p.timelinePlayers);

            p.audioActions->setTimelinePlayers(p.timelinePlayers);

            p.timelineViewport->setColorConfig(p.colorConfig);
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
            p.timelineViewport->setTimelinePlayers(p.timelinePlayers);

            p.timelineSlider->setColorConfig(p.colorConfig);
            p.timelineSlider->setTimelinePlayer(!p.timelinePlayers.empty() ? p.timelinePlayers[0] : nullptr);
            p.timelineSlider->setThumbnails(p.app->settingsObject()->value("Timeline/Thumbnails").toBool());
            p.timelineSlider->setStopOnScrub(p.app->settingsObject()->value("Timeline/StopOnScrub").toBool());

            p.compareTool->setCompareOptions(p.compareOptions);

            p.colorTool->setDisplayOptions(p.displayOptions);

            p.infoTool->setInfo(!p.timelinePlayers.empty() ? p.timelinePlayers[0]->ioInfo() : io::Info());

            p.audioTool->setAudioOffset(!p.timelinePlayers.empty() ? p.timelinePlayers[0]->audioOffset() : 0.0);

            std::vector<std::string> infoLabel;
            std::vector<std::string> infoTooltip;
            const int aIndex = p.app->filesModel()->observeAIndex()->get();
            if (count > 0 && aIndex >= 0 && aIndex < count)
            {
                const std::string fileName = files[aIndex]->path.get(-1, false);
                std::string fileNameLabel = fileName;
                if (fileNameLabel.size() > infoLabelMax)
                {
                    fileNameLabel.replace(infoLabelMax, fileNameLabel.size() - infoLabelMax, "...");
                }
                infoLabel.push_back(fileNameLabel);
                infoTooltip.push_back(fileName);

                const auto& ioInfo = files[aIndex]->ioInfo;
                if (!ioInfo.video.empty())
                {
                    {
                        std::stringstream ss;
                        ss << "V:" <<
                            ioInfo.video[0].size << " " <<
                            ioInfo.video[0].pixelType;
                        infoLabel.push_back(ss.str());
                    }
                    {
                        std::stringstream ss;
                        ss << "Video :" <<
                            ioInfo.video[0].size << " " <<
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
                p.secondaryWindow->viewport()->setColorConfig(p.colorConfig);
                p.secondaryWindow->viewport()->setImageOptions(imageOptions);
                p.secondaryWindow->viewport()->setDisplayOptions(displayOptions);
                p.secondaryWindow->viewport()->setCompareOptions(p.compareOptions);
                p.secondaryWindow->viewport()->setTimelinePlayers(p.timelinePlayers);
            }

            if (p.outputDevice)
            {
                p.outputDevice->setColorConfig(p.colorConfig);
                p.outputDevice->setImageOptions(imageOptions);
                p.outputDevice->setDisplayOptions(displayOptions);
                p.outputDevice->setCompareOptions(p.compareOptions);
                p.outputDevice->setTimelinePlayers(p.timelinePlayers);
            }
        }
    }
}
