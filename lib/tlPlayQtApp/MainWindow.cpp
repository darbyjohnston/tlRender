// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2023 Darby Johnston
// All rights reserved.

#include <tlPlayQtApp/MainWindow.h>

#include <tlPlayQtApp/App.h>
#include <tlPlayQtApp/AudioActions.h>
#include <tlPlayQtApp/AudioTool.h>
#include <tlPlayQtApp/ColorTool.h>
#include <tlPlayQtApp/CompareActions.h>
#include <tlPlayQtApp/DevicesModel.h>
#include <tlPlayQtApp/DevicesTool.h>
#include <tlPlayQtApp/FileActions.h>
#include <tlPlayQtApp/FilesTool.h>
#include <tlPlayQtApp/FrameActions.h>
#include <tlPlayQtApp/InfoTool.h>
#include <tlPlayQtApp/MessagesTool.h>
#include <tlPlayQtApp/OCIOModel.h>
#include <tlPlayQtApp/PlaybackActions.h>
#include <tlPlayQtApp/RenderActions.h>
#include <tlPlayQtApp/SettingsTool.h>
#include <tlPlayQtApp/SystemLogTool.h>
#include <tlPlayQtApp/TimelineActions.h>
#include <tlPlayQtApp/ToolActions.h>
#include <tlPlayQtApp/ViewActions.h>
#include <tlPlayQtApp/ViewTool.h>
#include <tlPlayQtApp/WindowActions.h>

#include <tlPlay/AudioModel.h>
#include <tlPlay/ColorModel.h>
#include <tlPlay/FilesModel.h>
#include <tlPlay/Info.h>
#include <tlPlay/Settings.h>

#include <tlQtWidget/Spacer.h>
#include <tlQtWidget/TimeLabel.h>
#include <tlQtWidget/TimeSpinBox.h>
#include <tlQtWidget/TimelineViewport.h>
#include <tlQtWidget/TimelineWidget.h>
#include <tlQtWidget/Util.h>

#include <tlQt/OutputDevice.h>

#include <tlCore/File.h>

#include <QComboBox>
#include <QDoubleSpinBox>
#include <QDragEnterEvent>
#include <QDragMoveEvent>
#include <QHBoxLayout>
#include <QLabel>
#include <QMenuBar>
#include <QMimeData>
#include <QMouseEvent>
#include <QSharedPointer>
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

            QVector<QSharedPointer<qt::TimelinePlayer> > timelinePlayers;
            bool floatOnTop = false;
            image::VideoLevels outputVideoLevels;

            FileActions* fileActions = nullptr;
            CompareActions* compareActions = nullptr;
            WindowActions* windowActions = nullptr;
            ViewActions* viewActions = nullptr;
            RenderActions* renderActions = nullptr;
            PlaybackActions* playbackActions = nullptr;
            FrameActions* frameActions = nullptr;
            TimelineActions* timelineActions = nullptr;
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
            ViewTool* viewTool = nullptr;
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

            std::shared_ptr<observer::ListObserver<std::shared_ptr<play::FilesModelItem> > > filesObserver;
            std::shared_ptr<observer::ValueObserver<int> > aIndexObserver;
            std::shared_ptr<observer::ListObserver<int> > bIndexesObserver;
            std::shared_ptr<observer::ValueObserver<timeline::CompareOptions> > compareOptionsObserver;
            std::shared_ptr<observer::ValueObserver<timeline::BackgroundOptions> > backgroundOptionsObserver;
            std::shared_ptr<observer::ValueObserver<timeline::OCIOOptions> > ocioOptionsObserver;
            std::shared_ptr<observer::ValueObserver<timeline::LUTOptions> > lutOptionsObserver;
            std::shared_ptr<observer::ValueObserver<timeline::DisplayOptions> > displayOptionsObserver;
            std::shared_ptr<observer::ValueObserver<timeline::ImageOptions> > imageOptionsObserver;
            std::shared_ptr<observer::ValueObserver<DevicesModelData> > devicesModelObserver;
            std::shared_ptr<observer::ValueObserver<float> > volumeObserver;
            std::shared_ptr<observer::ValueObserver<bool> > muteObserver;
            std::shared_ptr<observer::ListObserver<log::Item> > logObserver;
        };

        MainWindow::MainWindow(App* app, QWidget* parent) :
            QMainWindow(parent),
            _p(new Private)
        {
            TLRENDER_P();

            p.app = app;

            auto settings = app->settings();
            settings->setDefaultValue("MainWindow/Size", math::Size2i(1920, 1080));
            settings->setDefaultValue("MainWindow/FloatOnTop", false);
            settings->setDefaultValue("Timeline/Editable", true);
            settings->setDefaultValue("Timeline/EditAssociatedClips",
                timelineui::ItemOptions().editAssociatedClips);
            settings->setDefaultValue("Timeline/FrameView", true);
            settings->setDefaultValue("Timeline/StopOnScrub", true);
            settings->setDefaultValue("Timeline/Thumbnails",
                timelineui::ItemOptions().thumbnails);
            settings->setDefaultValue("Timeline/ThumbnailsSize",
                timelineui::ItemOptions().thumbnailHeight);
            settings->setDefaultValue("Timeline/Transitions",
                timelineui::ItemOptions().showTransitions);
            settings->setDefaultValue("Timeline/Markers",
                timelineui::ItemOptions().showMarkers);

            setAttribute(Qt::WA_DeleteOnClose);
            setFocusPolicy(Qt::StrongFocus);
            setAcceptDrops(true);

            p.floatOnTop = settings->getValue<bool>("MainWindow/FloatOnTop");

            auto context = app->getContext();
            p.timelineViewport = new qtwidget::TimelineViewport(context);

            p.timelineWidget = new qtwidget::TimelineWidget(
                ui::Style::create(context),
                app->timeUnitsModel(),
                context);
            p.timelineWidget->setEditable(settings->getValue<bool>("Timeline/Editable"));
            p.timelineWidget->setFrameView(settings->getValue<bool>("Timeline/FrameView"));
            p.timelineWidget->setScrollBarsVisible(false);
            p.timelineWidget->setStopOnScrub(settings->getValue<bool>("Timeline/StopOnScrub"));
            timelineui::ItemOptions itemOptions;
            itemOptions.editAssociatedClips = settings->getValue<bool>("Timeline/EditAssociatedClips");
            itemOptions.thumbnails = settings->getValue<bool>("Timeline/Thumbnails");
            itemOptions.thumbnailHeight = settings->getValue<int>("Timeline/ThumbnailsSize");
            itemOptions.showTransitions = settings->getValue<bool>("Timeline/Transitions");
            itemOptions.showMarkers = settings->getValue<bool>("Timeline/Markers");
            p.timelineWidget->setItemOptions(itemOptions);

            p.fileActions = new FileActions(app, this);
            p.compareActions = new CompareActions(app, this);
            p.windowActions = new WindowActions(app, this);
            p.viewActions = new ViewActions(app, this);
            p.renderActions = new RenderActions(app, this);
            p.playbackActions = new PlaybackActions(app, this);
            p.frameActions = new FrameActions(app, this);
            p.timelineActions = new TimelineActions(this, this);
            p.audioActions = new AudioActions(app, this);
            p.toolActions = new ToolActions(app, this);

            auto menuBar = new QMenuBar;
            menuBar->addMenu(p.fileActions->menu());
            menuBar->addMenu(p.compareActions->menu());
            menuBar->addMenu(p.windowActions->menu());
            menuBar->addMenu(p.viewActions->menu());
            menuBar->addMenu(p.renderActions->menu());
            menuBar->addMenu(p.playbackActions->menu());
            menuBar->addMenu(p.frameActions->menu());
            menuBar->addMenu(p.timelineActions->menu());
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

            setCentralWidget(p.timelineViewport);

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
            bottomToolBar->addAction(p.frameActions->actions()["Start"]);
            bottomToolBar->addAction(p.frameActions->actions()["FramePrev"]);
            bottomToolBar->addAction(p.frameActions->actions()["FrameNext"]);
            bottomToolBar->addAction(p.frameActions->actions()["End"]);
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

            p.filesTool = new FilesTool(app);
            auto filesDockWidget = new FilesDockWidget(p.filesTool);
            filesDockWidget->hide();
            p.toolActions->menu()->addAction(filesDockWidget->toggleViewAction());
            toolsToolBar->addAction(filesDockWidget->toggleViewAction());
            addDockWidget(Qt::RightDockWidgetArea, filesDockWidget);

            p.viewTool = new ViewTool(app);
            auto viewDockWidget = new ViewDockWidget(p.viewTool);
            viewDockWidget->hide();
            p.toolActions->menu()->addAction(viewDockWidget->toggleViewAction());
            toolsToolBar->addAction(viewDockWidget->toggleViewAction());
            addDockWidget(Qt::RightDockWidgetArea, viewDockWidget);

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

            _playersUpdate(app->players());
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
                [this](const timeline::CompareOptions&)
                {
                    _widgetUpdate();
                });

            p.backgroundOptionsObserver = observer::ValueObserver<timeline::BackgroundOptions>::create(
                app->viewportModel()->observeBackgroundOptions(),
                [this](const timeline::BackgroundOptions& value)
                {
                    _widgetUpdate();
                });

            p.ocioOptionsObserver = observer::ValueObserver<timeline::OCIOOptions>::create(
                app->colorModel()->observeOCIOOptions(),
                [this](const timeline::OCIOOptions&)
                {
                    _widgetUpdate();
                });
            p.lutOptionsObserver = observer::ValueObserver<timeline::LUTOptions>::create(
                app->colorModel()->observeLUTOptions(),
                [this](const timeline::LUTOptions&)
                {
                    _widgetUpdate();
                });
            p.displayOptionsObserver = observer::ValueObserver<timeline::DisplayOptions>::create(
                app->colorModel()->observeDisplayOptions(),
                [this](const timeline::DisplayOptions&)
                {
                    _widgetUpdate();
                });
            p.imageOptionsObserver = observer::ValueObserver<timeline::ImageOptions>::create(
                app->colorModel()->observeImageOptions(),
                [this](const timeline::ImageOptions&)
                {
                    _widgetUpdate();
                });

            p.devicesModelObserver = observer::ValueObserver<DevicesModelData>::create(
                app->devicesModel()->observeData(),
                [this](const DevicesModelData& value)
                {
                    _p->outputVideoLevels = value.videoLevels;
                    _widgetUpdate();
                });

            p.volumeObserver = observer::ValueObserver<float>::create(
                app->audioModel()->observeVolume(),
                [this](float)
                {
                    _widgetUpdate();
                });
            p.muteObserver = observer::ValueObserver<bool>::create(
                app->audioModel()->observeMute(),
                [this](bool)
                {
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
                p.windowActions,
                &WindowActions::resize,
                [this](const image::Size& size)
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
                    _widgetUpdate();
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
                p.frameActions->actions()["FocusCurrentFrame"],
                &QAction::triggered,
                [this]
                {
                    _p->currentTimeSpinBox->setFocus(Qt::OtherFocusReason);
                    _p->currentTimeSpinBox->selectAll();
                });

            connect(
                p.timelineWidget,
                &qtwidget::TimelineWidget::editableChanged,
                [this](bool value)
                {
                    _p->timelineActions->actions()["Editable"]->setChecked(value);
                });
            connect(
                p.timelineWidget,
                &qtwidget::TimelineWidget::frameViewChanged,
                [this](bool value)
                {
                    _p->timelineActions->actions()["FrameView"]->setChecked(value);
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
                        _p->currentTimeSpinBox->setValue(_p->timelinePlayers[0]->currentTime());
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
                p.timelineViewport,
                &qtwidget::TimelineViewport::compareOptionsChanged,
                [this](const timeline::CompareOptions& value)
                {
                    _p->app->filesModel()->setCompareOptions(value);
                });
            connect(
                p.timelineViewport,
                &qtwidget::TimelineViewport::viewPosAndZoomChanged,
                [this](const math::Vector2i& pos, float zoom)
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
                app,
                &App::activePlayersChanged,
                [this](const QVector<QSharedPointer<qt::TimelinePlayer> >& value)
                {
                    _playersUpdate(value);
                });

            connect(
                app->timeObject(),
                &qt::TimeObject::timeUnitsChanged,
                [this](timeline::TimeUnits value)
                {
                    _p->timeUnitsComboBox->setCurrentIndex(
                        static_cast<int>(value));
                });
        }

        MainWindow::~MainWindow()
        {
            TLRENDER_P();
            auto settings = p.app->settings();
            settings->setValue("MainWindow/Size", math::Size2i(width(), height()));
            settings->setValue("MainWindow/FloatOnTop", p.floatOnTop);
            settings->setValue("Timeline/Editable",
                p.timelineWidget->isEditable());
            const auto& timelineItemOptions = p.timelineWidget->itemOptions();
            settings->setValue("Timeline/EditAssociatedClips",
                timelineItemOptions.editAssociatedClips);
            settings->setValue("Timeline/FrameView",
                p.timelineWidget->hasFrameView());
            settings->setValue("Timeline/StopOnScrub",
                p.timelineWidget->hasStopOnScrub());
            settings->setValue("Timeline/Thumbnails",
                timelineItemOptions.thumbnails);
            settings->setValue("Timeline/ThumbnailsSize",
                timelineItemOptions.thumbnailHeight);
            settings->setValue("Timeline/Transitions",
                timelineItemOptions.showTransitions);
            settings->setValue("Timeline/Markers",
                timelineItemOptions.showMarkers);
        }

        qtwidget::TimelineWidget* MainWindow::timelineWidget() const
        {
            return _p->timelineWidget;
        }

        qtwidget::TimelineViewport* MainWindow::timelineViewport() const
        {
            return _p->timelineViewport;
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
            p.app->audioModel()->setVolume(value / static_cast<float>(sliderSteps));
        }

        void MainWindow::_playersUpdate(const QVector<QSharedPointer<qt::TimelinePlayer> >& timelinePlayers)
        {
            TLRENDER_P();
            if (!p.timelinePlayers.empty() && p.timelinePlayers[0])
            {
                disconnect(
                    p.timelinePlayers[0].get(),
                    SIGNAL(speedChanged(double)),
                    this,
                    SLOT(_speedCallback(double)));
                disconnect(
                    p.timelinePlayers[0].get(),
                    SIGNAL(playbackChanged(tl::timeline::Playback)),
                    this,
                    SLOT(_playbackCallback(tl::timeline::Playback)));
                disconnect(
                    p.timelinePlayers[0].get(),
                    SIGNAL(currentTimeChanged(const otime::RationalTime&)),
                    this,
                    SLOT(_currentTimeCallback(const otime::RationalTime&)));
            }

            p.timelinePlayers = timelinePlayers;

            if (!p.timelinePlayers.empty() && p.timelinePlayers[0])
            {
                connect(
                    p.timelinePlayers[0].get(),
                    SIGNAL(speedChanged(double)),
                    SLOT(_speedCallback(double)));
                connect(
                    p.timelinePlayers[0].get(),
                    SIGNAL(playbackChanged(tl::timeline::Playback)),
                    SLOT(_playbackCallback(tl::timeline::Playback)));
                connect(
                    p.timelinePlayers[0].get(),
                    SIGNAL(currentTimeChanged(const otime::RationalTime&)),
                    SLOT(_currentTimeCallback(const otime::RationalTime&)));
            }

            p.timelineViewport->setTimelinePlayers(p.timelinePlayers);

            _widgetUpdate();
        }

        void MainWindow::_widgetUpdate()
        {
            TLRENDER_P();

            qtwidget::setFloatOnTop(p.floatOnTop, this);

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

            p.viewActions->actions()["Frame"]->setChecked(
                p.timelineViewport->hasFrameView());

            auto viewportModel = p.app->viewportModel();
            p.timelineViewport->setBackgroundOptions(
                viewportModel->getBackgroundOptions());
            auto colorModel = p.app->colorModel();
            p.timelineViewport->setOCIOOptions(colorModel->getOCIOOptions());
            p.timelineViewport->setLUTOptions(colorModel->getLUTOptions());
            std::vector<timeline::ImageOptions> imageOptions;
            std::vector<timeline::DisplayOptions> displayOptions;
            for (const auto& i : p.timelinePlayers)
            {
                imageOptions.push_back(colorModel->getImageOptions());
                displayOptions.push_back(colorModel->getDisplayOptions());
            }
            p.timelineViewport->setImageOptions(imageOptions);
            p.timelineViewport->setDisplayOptions(displayOptions);
            p.timelineViewport->setCompareOptions(
                p.app->filesModel()->getCompareOptions());

            p.timelineWidget->setPlayer(
                (!p.timelinePlayers.empty() && p.timelinePlayers[0]) ?
                p.timelinePlayers[0]->player() :
                nullptr);

            {
                const QSignalBlocker blocker(p.volumeSlider);
                const float volume = p.app->audioModel()->getVolume();
                p.volumeSlider->setValue(volume * sliderSteps);
            }

            p.infoTool->setInfo(
                (!p.timelinePlayers.empty() && p.timelinePlayers[0]) ?
                p.timelinePlayers[0]->ioInfo() :
                io::Info());

            std::string infoLabel;
            std::string infoToolTip;
            if (!p.timelinePlayers.empty() && p.timelinePlayers[0])
            {
                const file::Path& path = p.timelinePlayers[0]->path();
                const io::Info& ioInfo = p.timelinePlayers[0]->ioInfo();
                infoLabel = play::infoLabel(path, ioInfo);
                infoToolTip = play::infoToolTip(path, ioInfo);
            }
            p.infoLabel->setText(QString::fromUtf8(infoLabel.c_str()));
            p.infoLabel->setToolTip(QString::fromUtf8(infoToolTip.c_str()));

            p.app->outputDevice()->setOCIOOptions(colorModel->getOCIOOptions());
            p.app->outputDevice()->setLUTOptions(colorModel->getLUTOptions());
            p.app->outputDevice()->setImageOptions(imageOptions);
            for (auto& i : displayOptions)
            {
                i.videoLevels = p.outputVideoLevels;
            }
            p.app->outputDevice()->setDisplayOptions(displayOptions);
            p.app->outputDevice()->setCompareOptions(p.app->filesModel()->getCompareOptions());
        }
    }
}
