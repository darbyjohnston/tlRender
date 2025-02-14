// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2025 Darby Johnston
// All rights reserved.

#include <tlPlayQtApp/MainWindow.h>

#include <tlPlayQtApp/App.h>
#include <tlPlayQtApp/AudioActions.h>
#include <tlPlayQtApp/AudioTool.h>
#include <tlPlayQtApp/ColorTool.h>
#include <tlPlayQtApp/CompareActions.h>
#include <tlPlayQtApp/DevicesTool.h>
#include <tlPlayQtApp/FileActions.h>
#include <tlPlayQtApp/FilesTool.h>
#include <tlPlayQtApp/FrameActions.h>
#include <tlPlayQtApp/InfoTool.h>
#include <tlPlayQtApp/MessagesTool.h>
#include <tlPlayQtApp/PlaybackActions.h>
#include <tlPlayQtApp/RenderActions.h>
#include <tlPlayQtApp/SettingsTool.h>
#include <tlPlayQtApp/SystemLogTool.h>
#include <tlPlayQtApp/TimelineActions.h>
#include <tlPlayQtApp/ToolActions.h>
#include <tlPlayQtApp/ViewActions.h>
#include <tlPlayQtApp/ViewTool.h>
#include <tlPlayQtApp/WindowActions.h>

#include <tlQtWidget/ContainerWidget.h>
#include <tlQtWidget/Spacer.h>
#include <tlQtWidget/TimeLabel.h>
#include <tlQtWidget/TimeSpinBox.h>
#include <tlQtWidget/TimelineWidget.h>
#include <tlQtWidget/Util.h>

#include <tlPlay/AudioModel.h>
#include <tlPlay/ColorModel.h>
#include <tlPlay/FilesModel.h>
#include <tlPlay/Info.h>
#include <tlPlay/RenderModel.h>
#include <tlPlay/Viewport.h>

#if defined(TLRENDER_BMD)
#include <tlDevice/BMDOutputDevice.h>
#endif // TLRENDER_BMD

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

            QSharedPointer<qt::TimelinePlayer> player;
            bool floatOnTop = false;

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

            std::shared_ptr<play::Viewport> viewport = nullptr;
            qtwidget::ContainerWidget* viewportContainer = nullptr;
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

            std::shared_ptr<dtk::ListObserver<std::shared_ptr<play::FilesModelItem> > > filesObserver;
            std::shared_ptr<dtk::ValueObserver<int> > aIndexObserver;
            std::shared_ptr<dtk::ListObserver<int> > bIndexesObserver;
            std::shared_ptr<dtk::ValueObserver<timeline::CompareOptions> > compareOptionsObserver;
            std::shared_ptr<dtk::ValueObserver<timeline::OCIOOptions> > ocioOptionsObserver;
            std::shared_ptr<dtk::ValueObserver<timeline::LUTOptions> > lutOptionsObserver;
            std::shared_ptr<dtk::ValueObserver<timeline::DisplayOptions> > displayOptionsObserver;
            std::shared_ptr<dtk::ValueObserver<dtk::ImageOptions> > imageOptionsObserver;
            std::shared_ptr<dtk::ValueObserver<timeline::BackgroundOptions> > backgroundOptionsObserver;
            std::shared_ptr<dtk::ValueObserver<dtk::ImageType> > colorBufferObserver;
            std::shared_ptr<dtk::ValueObserver<float> > volumeObserver;
            std::shared_ptr<dtk::ValueObserver<bool> > muteObserver;
            std::shared_ptr<dtk::ListObserver<dtk::LogItem> > logObserver;
        };

        MainWindow::MainWindow(App* app, QWidget* parent) :
            QMainWindow(parent),
            _p(new Private)
        {
            DTK_P();

            p.app = app;

            setAttribute(Qt::WA_DeleteOnClose);
            setFocusPolicy(Qt::StrongFocus);
            setAcceptDrops(true);

            auto context = app->getContext();
            p.viewport = play::Viewport::create(context);
            auto style = dtk::Style::create(context);
            p.viewportContainer = new qtwidget::ContainerWidget(context, style);
            p.viewportContainer->setWidget(p.viewport);

            p.timelineWidget = new qtwidget::TimelineWidget(
                context,
                app->timeUnitsModel(),
                style);
            const play::TimelineOptions timelineOptions = app->settingsModel()->getTimeline();
            p.timelineWidget->setEditable(timelineOptions.editable);
            p.timelineWidget->setFrameView(timelineOptions.frameView);
            p.timelineWidget->setScrollBarsVisible(false);
            p.timelineWidget->setScrollToCurrentFrame(timelineOptions.scroll);
            p.timelineWidget->setStopOnScrub(timelineOptions.stopOnScrub);
            p.timelineWidget->setItemOptions(app->settingsModel()->getTimelineItem());
            timelineui::DisplayOptions timeineDisplayOptions = app->settingsModel()->getTimelineDisplay();
            if (app->settingsModel()->getTimelineFirstTrack())
            {
                timeineDisplayOptions.tracks = { 0 };
            }
            timeineDisplayOptions.waveformHeight = timeineDisplayOptions.thumbnailHeight / 2;
            p.timelineWidget->setDisplayOptions(timeineDisplayOptions);

            p.fileActions = new FileActions(app, this);
            p.compareActions = new CompareActions(app, this);
            p.windowActions = new WindowActions(app, this);
            p.viewActions = new ViewActions(app, this, this);
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
            viewToolBar->addAction(p.viewActions->actions()["ZoomReset"]);
            addToolBar(Qt::TopToolBarArea, viewToolBar);

            auto toolsToolBar = new QToolBar;
            toolsToolBar->setObjectName("ToolsToolBar");
            toolsToolBar->setWindowTitle("Tools Tool Bar");
            toolsToolBar->setIconSize(QSize(20, 20));
            toolsToolBar->setAllowedAreas(Qt::TopToolBarArea);
            toolsToolBar->setFloatable(false);
            addToolBar(Qt::TopToolBarArea, toolsToolBar);

            setCentralWidget(p.viewportContainer);

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

            p.viewportContainer->setFocus();

            _playerUpdate(app->player());
            _widgetUpdate();

            p.filesObserver = dtk::ListObserver<std::shared_ptr<play::FilesModelItem> >::create(
                app->filesModel()->observeFiles(),
                [this](const std::vector<std::shared_ptr<play::FilesModelItem> >&)
                {
                    _widgetUpdate();
                });
            p.aIndexObserver = dtk::ValueObserver<int>::create(
                app->filesModel()->observeAIndex(),
                [this](int)
                {
                    _widgetUpdate();
                });
            p.bIndexesObserver = dtk::ListObserver<int>::create(
                app->filesModel()->observeBIndexes(),
                [this](const std::vector<int>&)
                {
                    _widgetUpdate();
                });
            p.compareOptionsObserver = dtk::ValueObserver<timeline::CompareOptions>::create(
                app->filesModel()->observeCompareOptions(),
                [this](const timeline::CompareOptions&)
                {
                    _widgetUpdate();
                });

            p.ocioOptionsObserver = dtk::ValueObserver<timeline::OCIOOptions>::create(
                app->colorModel()->observeOCIOOptions(),
                [this](const timeline::OCIOOptions&)
                {
                    _widgetUpdate();
                });
            p.lutOptionsObserver = dtk::ValueObserver<timeline::LUTOptions>::create(
                app->colorModel()->observeLUTOptions(),
                [this](const timeline::LUTOptions&)
                {
                    _widgetUpdate();
                });

            p.displayOptionsObserver = dtk::ValueObserver<timeline::DisplayOptions>::create(
                app->viewportModel()->observeDisplayOptions(),
                [this](const timeline::DisplayOptions&)
                {
                    _widgetUpdate();
                });
            p.backgroundOptionsObserver = dtk::ValueObserver<timeline::BackgroundOptions>::create(
                app->viewportModel()->observeBackgroundOptions(),
                [this](const timeline::BackgroundOptions& value)
                {
                    _widgetUpdate();
                });

            p.imageOptionsObserver = dtk::ValueObserver<dtk::ImageOptions>::create(
                app->renderModel()->observeImageOptions(),
                [this](const dtk::ImageOptions&)
                {
                    _widgetUpdate();
                });
            p.colorBufferObserver = dtk::ValueObserver<dtk::ImageType>::create(
                app->renderModel()->observeColorBuffer(),
                [this](dtk::ImageType)
                {
                    _widgetUpdate();
                });

            p.volumeObserver = dtk::ValueObserver<float>::create(
                app->audioModel()->observeVolume(),
                [this](float)
                {
                    _widgetUpdate();
                });
            p.muteObserver = dtk::ValueObserver<bool>::create(
                app->audioModel()->observeMute(),
                [this](bool)
                {
                    _widgetUpdate();
                });

            p.logObserver = dtk::ListObserver<dtk::LogItem>::create(
                context->getLogSystem()->observeLogItems(),
                [this](const std::vector<dtk::LogItem>& value)
                {
                    for (const auto& i : value)
                    {
                        switch (i.type)
                        {
                        case dtk::LogType::Error:
                            _p->statusBar->showMessage(
                                QString::fromUtf8(dtk::toString(i).c_str()),
                                errorTimeout);
                            break;
                        default: break;
                        }
                    }
                });

            connect(
                p.windowActions,
                &WindowActions::resize,
                [this](const dtk::Size2I& size)
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
                    _p->viewport->setFrameView(value);
                });
            connect(
                p.viewActions->actions()["ZoomReset"],
                &QAction::triggered,
                [this]
                {
                    _p->viewport->viewZoomReset();
                });
            connect(
                p.viewActions->actions()["ZoomIn"],
                &QAction::triggered,
                [this]
                {
                    _p->viewport->viewZoomIn();
                });
            connect(
                p.viewActions->actions()["ZoomOut"],
                &QAction::triggered,
                [this]
                {
                    _p->viewport->viewZoomOut();
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
            /*connect(
                p.timelineWidget,
                &qtwidget::TimelineWidget::timeScrubbed,
                [this](const OTIO_NS::RationalTime& value)
                {
                    std::cout << "Time scrubbed: " << value << std::endl;
                });*/

            connect(
                p.currentTimeSpinBox,
                &qtwidget::TimeSpinBox::valueChanged,
                [this](const OTIO_NS::RationalTime& value)
                {
                    if (_p->player)
                    {
                        _p->player->setPlayback(timeline::Playback::Stop);
                        _p->player->seek(value);
                        _p->currentTimeSpinBox->setValue(_p->player->currentTime());
                    }
                });

            connect(
                p.speedSpinBox,
                QOverload<double>::of(&QDoubleSpinBox::valueChanged),
                [this](double value)
                {
                    if (_p->player)
                    {
                        _p->player->setSpeed(value);
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

            p.viewport->setCompareCallback(
                [this](const timeline::CompareOptions& value)
                {
                    _p->app->filesModel()->setCompareOptions(value);
                });

            connect(
                app,
                &App::playerChanged,
                [this](const QSharedPointer<qt::TimelinePlayer>& value)
                {
                    _playerUpdate(value);
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
        {}

        const std::shared_ptr<play::Viewport>& MainWindow::viewport() const
        {
            return _p->viewport;
        }

        qtwidget::TimelineWidget* MainWindow::timelineWidget() const
        {
            return _p->timelineWidget;
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
            DTK_P();
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

        void MainWindow::_currentTimeCallback(const OTIO_NS::RationalTime& value)
        {
            DTK_P();
            const QSignalBlocker blocker(p.currentTimeSpinBox);
            p.currentTimeSpinBox->setValue(value);
        }

        void MainWindow::_volumeCallback(int value)
        {
            DTK_P();
            p.app->audioModel()->setVolume(value / static_cast<float>(sliderSteps));
        }

        void MainWindow::_playerUpdate(const QSharedPointer<qt::TimelinePlayer>& player)
        {
            DTK_P();
            if (p.player)
            {
                disconnect(
                    p.player.get(),
                    SIGNAL(speedChanged(double)),
                    this,
                    SLOT(_speedCallback(double)));
                disconnect(
                    p.player.get(),
                    SIGNAL(playbackChanged(tl::timeline::Playback)),
                    this,
                    SLOT(_playbackCallback(tl::timeline::Playback)));
                disconnect(
                    p.player.get(),
                    SIGNAL(currentTimeChanged(const OTIO_NS::RationalTime&)),
                    this,
                    SLOT(_currentTimeCallback(const OTIO_NS::RationalTime&)));
            }

            p.player = player;

            if (p.player)
            {
                connect(
                    p.player.get(),
                    SIGNAL(speedChanged(double)),
                    SLOT(_speedCallback(double)));
                connect(
                    p.player.get(),
                    SIGNAL(playbackChanged(tl::timeline::Playback)),
                    SLOT(_playbackCallback(tl::timeline::Playback)));
                connect(
                    p.player.get(),
                    SIGNAL(currentTimeChanged(const OTIO_NS::RationalTime&)),
                    SLOT(_currentTimeCallback(const OTIO_NS::RationalTime&)));
            }

            p.viewport->setPlayer(p.player ? p.player->player() : nullptr);

            _widgetUpdate();
        }

        void MainWindow::_widgetUpdate()
        {
            DTK_P();

            qtwidget::setFloatOnTop(p.floatOnTop, this);

            const auto& files = p.app->filesModel()->observeFiles()->get();
            const size_t count = files.size();
            p.timelineWidget->setEnabled(count > 0);
            p.currentTimeSpinBox->setEnabled(count > 0);
            p.speedSpinBox->setEnabled(count > 0);
            p.volumeSlider->setEnabled(count > 0);

            if (p.player)
            {
                {
                    QSignalBlocker blocker(p.currentTimeSpinBox);
                    p.currentTimeSpinBox->setValue(p.player->currentTime());
                }

                {
                    QSignalBlocker blocker(p.speedSpinBox);
                    p.speedSpinBox->setValue(p.player->speed());
                }

                const auto& timeRange = p.player->timeRange();
                p.durationLabel->setValue(timeRange.duration());

                {
                    QSignalBlocker blocker(p.volumeSlider);
                    p.volumeSlider->setValue(p.player->volume() * sliderSteps);
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
                p.viewport->hasFrameView());

            p.viewport->setCompareOptions(
                p.app->filesModel()->getCompareOptions());
            p.viewport->setOCIOOptions(
                p.app->colorModel()->getOCIOOptions());
            p.viewport->setLUTOptions(
                p.app->colorModel()->getLUTOptions());
            p.viewport->setDisplayOptions(
                { p.app->viewportModel()->getDisplayOptions()});
            p.viewport->setBackgroundOptions(
                p.app->viewportModel()->getBackgroundOptions());
            p.viewport->setImageOptions(
                { p.app->renderModel()->getImageOptions() });
            p.viewport->setColorBuffer(
                p.app->renderModel()->getColorBuffer());

            auto displayOptions = p.timelineWidget->displayOptions();
            displayOptions.ocio = p.app->colorModel()->getOCIOOptions();
            displayOptions.lut = p.app->colorModel()->getLUTOptions();
            p.timelineWidget->setDisplayOptions(displayOptions);
            p.timelineWidget->setPlayer(p.player ? p.player->player() : nullptr);

            {
                const QSignalBlocker blocker(p.volumeSlider);
                const float volume = p.app->audioModel()->getVolume();
                p.volumeSlider->setValue(volume * sliderSteps);
            }

            p.infoTool->setInfo(p.player ? p.player->ioInfo() : io::Info());

            std::string infoLabel;
            std::string infoToolTip;
            if (p.player)
            {
                const file::Path& path = p.player->path();
                const io::Info& ioInfo = p.player->ioInfo();
                infoLabel = play::infoLabel(path, ioInfo);
                infoToolTip = play::infoToolTip(path, ioInfo);
            }
            p.infoLabel->setText(QString::fromUtf8(infoLabel.c_str()));
            p.infoLabel->setToolTip(QString::fromUtf8(infoToolTip.c_str()));
        }
    }
}
