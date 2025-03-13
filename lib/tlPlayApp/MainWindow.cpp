// SPDX-License-Identifier: BSD-3-Clause
// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2025 Darby Johnston
// All rights reserved.

#include <tlPlayApp/MainWindow.h>

#include <tlPlayApp/Actions/AudioActions.h>
#include <tlPlayApp/Actions/CompareActions.h>
#include <tlPlayApp/Actions/FileActions.h>
#include <tlPlayApp/Actions/FrameActions.h>
#include <tlPlayApp/Actions/PlaybackActions.h>
#include <tlPlayApp/Actions/TimelineActions.h>
#include <tlPlayApp/Actions/ToolsActions.h>
#include <tlPlayApp/Actions/ViewActions.h>
#include <tlPlayApp/Actions/WindowActions.h>
#include <tlPlayApp/Menus/AudioMenu.h>
#include <tlPlayApp/Menus/CompareMenu.h>
#include <tlPlayApp/Menus/FileMenu.h>
#include <tlPlayApp/Menus/FrameMenu.h>
#include <tlPlayApp/Menus/PlaybackMenu.h>
#include <tlPlayApp/Menus/TimelineMenu.h>
#include <tlPlayApp/Menus/ToolsMenu.h>
#include <tlPlayApp/Menus/ViewMenu.h>
#include <tlPlayApp/Menus/WindowMenu.h>
#include <tlPlayApp/Models/AudioModel.h>
#include <tlPlayApp/Models/ColorModel.h>
#include <tlPlayApp/Models/TimeUnitsModel.h>
#include <tlPlayApp/Models/ViewportModel.h>
#include <tlPlayApp/Tools/ToolsWidget.h>
#include <tlPlayApp/Widgets/AudioPopup.h>
#include <tlPlayApp/Widgets/CompareToolBar.h>
#include <tlPlayApp/Widgets/FileToolBar.h>
#include <tlPlayApp/Widgets/SpeedPopup.h>
#include <tlPlayApp/Widgets/StatusBar.h>
#include <tlPlayApp/Widgets/ToolsToolBar.h>
#include <tlPlayApp/Widgets/ViewToolBar.h>
#include <tlPlayApp/Widgets/Viewport.h>
#include <tlPlayApp/Widgets/WindowToolBar.h>
#include <tlPlayApp/App.h>

#include <tlTimelineUI/TimeEdit.h>
#include <tlTimelineUI/TimeLabel.h>
#include <tlTimelineUI/TimelineWidget.h>

#include <tlTimelineGL/Render.h>

#include <dtk/ui/ButtonGroup.h>
#include <dtk/ui/ComboBox.h>
#include <dtk/ui/Divider.h>
#include <dtk/ui/DoubleEdit.h>
#include <dtk/ui/DoubleModel.h>
#include <dtk/ui/Label.h>
#include <dtk/ui/Menu.h>
#include <dtk/ui/MenuBar.h>
#include <dtk/ui/RowLayout.h>
#include <dtk/ui/Spacer.h>
#include <dtk/ui/Splitter.h>
#include <dtk/ui/ToolButton.h>

#if defined(TLRENDER_BMD)
#include <tlDevice/BMDOutputDevice.h>
#endif // TLRENDER_BMD

namespace tl
{
    namespace play
    {

        struct MainWindow::Private
        {
            std::weak_ptr<App> app;
            std::shared_ptr<SettingsModel> settingsModel;
            std::shared_ptr<dtk::DoubleModel> speedModel;
            timelineui::ItemOptions itemOptions;
            std::shared_ptr<timeline::Player> player;

            std::shared_ptr<Viewport> viewport;
            std::shared_ptr<timelineui::TimelineWidget> timelineWidget;
            std::shared_ptr<FileActions> fileActions;
            std::shared_ptr<CompareActions> compareActions;
            std::shared_ptr<WindowActions> windowActions;
            std::shared_ptr<ViewActions> viewActions;
            std::shared_ptr<PlaybackActions> playbackActions;
            std::shared_ptr<FrameActions> frameActions;
            std::shared_ptr<TimelineActions> timelineActions;
            std::shared_ptr<AudioActions> audioActions;
            std::shared_ptr<ToolsActions> toolsActions;
            std::shared_ptr<FileMenu> fileMenu;
            std::shared_ptr<CompareMenu> compareMenu;
            std::shared_ptr<WindowMenu> windowMenu;
            std::shared_ptr<ViewMenu> viewMenu;
            std::shared_ptr<PlaybackMenu> playbackMenu;
            std::shared_ptr<FrameMenu> frameMenu;
            std::shared_ptr<TimelineMenu> timelineMenu;
            std::shared_ptr<AudioMenu> audioMenu;
            std::shared_ptr<ToolsMenu> toolsMenu;
            std::shared_ptr<dtk::MenuBar> menuBar;
            std::shared_ptr<FileToolBar> fileToolBar;
            std::shared_ptr<CompareToolBar> compareToolBar;
            std::shared_ptr<WindowToolBar> windowToolBar;
            std::shared_ptr<ViewToolBar> viewToolBar;
            std::shared_ptr<ToolsToolBar> toolsToolBar;
            std::shared_ptr<timelineui::TimeEdit> currentTimeEdit;
            std::shared_ptr<timelineui::TimeLabel> durationLabel;
            std::shared_ptr<dtk::ComboBox> timeUnitsComboBox;
            std::shared_ptr<dtk::DoubleEdit> speedEdit;
            std::shared_ptr<dtk::ToolButton> speedButton;
            std::shared_ptr<SpeedPopup> speedPopup;
            std::shared_ptr<dtk::ToolButton> audioButton;
            std::shared_ptr<AudioPopup> audioPopup;
            std::shared_ptr<dtk::ToolButton> muteButton;
            std::shared_ptr<StatusBar> statusBar;
            std::shared_ptr<ToolsWidget> toolsWidget;
            std::map<std::string, std::shared_ptr<dtk::Divider> > dividers;
            std::shared_ptr<dtk::Splitter> splitter;
            std::shared_ptr<dtk::Splitter> splitter2;
            std::shared_ptr<dtk::HorizontalLayout> bottomLayout;
            std::shared_ptr<dtk::VerticalLayout> layout;

            std::shared_ptr<dtk::ValueObserver<std::shared_ptr<timeline::Player> > > playerObserver;
            std::shared_ptr<dtk::ValueObserver<double> > speedObserver;
            std::shared_ptr<dtk::ValueObserver<double> > speedObserver2;
            std::shared_ptr<dtk::ValueObserver<OTIO_NS::RationalTime> > currentTimeObserver;
            std::shared_ptr<dtk::ValueObserver<timeline::OCIOOptions> > ocioOptionsObserver;
            std::shared_ptr<dtk::ValueObserver<timeline::LUTOptions> > lutOptionsObserver;
            std::shared_ptr<dtk::ValueObserver<dtk::ImageType> > colorBufferObserver;
            std::shared_ptr<dtk::ValueObserver<bool> > muteObserver;
            std::shared_ptr<dtk::ValueObserver<TimelineSettings> > timelineSettingsObserver;
            std::shared_ptr<dtk::ValueObserver<WindowSettings> > windowSettingsObserver;
        };

        void MainWindow::_init(
            const std::shared_ptr<dtk::Context>& context,
            const std::shared_ptr<App>& app)
        {
            const WindowSettings& settings = app->getSettingsModel()->getWindow();
            Window::_init(context, "tlplay", settings.size);
            DTK_P();

            setBackgroundRole(dtk::ColorRole::Window);

            p.app = app;
            p.settingsModel = app->getSettingsModel();
            p.speedModel = dtk::DoubleModel::create(context);
            p.speedModel->setRange(dtk::RangeD(0.0, 1000000.0));
            p.speedModel->setStep(1.F);
            p.speedModel->setLargeStep(10.F);

            p.viewport = Viewport::create(context, app);

            auto timeUnitsModel = app->getTimeUnitsModel();
            p.timelineWidget = timelineui::TimelineWidget::create(context, timeUnitsModel);
            p.timelineWidget->setScrollBarsVisible(false);

            p.fileActions = FileActions::create(context, app);
            p.compareActions = CompareActions::create(context, app);
            p.windowActions = WindowActions::create(
                context,
                app,
                std::dynamic_pointer_cast<MainWindow>(shared_from_this()));
            p.viewActions = ViewActions::create(
                context,
                app,
                std::dynamic_pointer_cast<MainWindow>(shared_from_this()));
            p.playbackActions = PlaybackActions::create(context, app);
            p.frameActions = FrameActions::create(
                context,
                app,
                std::dynamic_pointer_cast<MainWindow>(shared_from_this()));
            p.timelineActions = TimelineActions::create(
                context,
                app,
                std::dynamic_pointer_cast<MainWindow>(shared_from_this()));
            p.audioActions = AudioActions::create(context, app);
            p.toolsActions = ToolsActions::create(context, app);

            p.fileMenu = FileMenu::create(context, app, p.fileActions);
            p.compareMenu = CompareMenu::create(context, app, p.compareActions);
            p.windowMenu = WindowMenu::create(
                context,
                app,
                std::dynamic_pointer_cast<MainWindow>(shared_from_this()),
                p.windowActions);
            p.viewMenu = ViewMenu::create(context, p.viewActions);
            p.playbackMenu = PlaybackMenu::create(context, app, p.playbackActions);
            p.frameMenu = FrameMenu::create(context, app, p.frameActions);
            p.timelineMenu = TimelineMenu::create(
                context,
                app,
                std::dynamic_pointer_cast<MainWindow>(shared_from_this()),
                p.timelineActions);
            p.audioMenu = AudioMenu::create(context, app, p.audioActions);
            p.toolsMenu = ToolsMenu::create(context, app, p.toolsActions);
            p.menuBar = dtk::MenuBar::create(context);
            p.menuBar->addMenu("File", p.fileMenu);
            p.menuBar->addMenu("Compare", p.compareMenu);
            p.menuBar->addMenu("Window", p.windowMenu);
            p.menuBar->addMenu("View", p.viewMenu);
            p.menuBar->addMenu("Playback", p.playbackMenu);
            p.menuBar->addMenu("Frame", p.frameMenu);
            p.menuBar->addMenu("Timeline", p.timelineMenu);
            p.menuBar->addMenu("Audio", p.audioMenu);
            p.menuBar->addMenu("Tools", p.toolsMenu);

            p.fileToolBar = FileToolBar::create(
                context,
                app,
                p.fileActions->getActions());
            p.compareToolBar = CompareToolBar::create(
                context,
                app,
                p.compareActions->getActions());
            p.windowToolBar = WindowToolBar::create(
                context,
                app,
                std::dynamic_pointer_cast<MainWindow>(shared_from_this()),
                p.windowActions->getActions());
            p.viewToolBar = ViewToolBar::create(context, p.viewActions);
            p.toolsToolBar = ToolsToolBar::create(
                context,
                app,
                p.toolsActions->getActions());

            auto playbackActions = p.playbackActions->getActions();
            auto stopButton = dtk::ToolButton::create(context, playbackActions["Stop"]);
            auto forwardButton = dtk::ToolButton::create(context, playbackActions["Forward"]);
            auto reverseButton = dtk::ToolButton::create(context, playbackActions["Reverse"]);

            auto frameActions = p.frameActions->getActions();
            auto timeStartButton = dtk::ToolButton::create(context, frameActions["Start"]);
            auto timeEndButton = dtk::ToolButton::create(context, frameActions["End"]);
            auto framePrevButton = dtk::ToolButton::create(context, frameActions["Prev"]);
            framePrevButton->setRepeatClick(true);
            auto frameNextButton = dtk::ToolButton::create(context, frameActions["Next"]);
            frameNextButton->setRepeatClick(true);

            p.currentTimeEdit = timelineui::TimeEdit::create(context, timeUnitsModel);
            p.currentTimeEdit->setTooltip("Current time");

            p.durationLabel = timelineui::TimeLabel::create(context, timeUnitsModel);
            p.durationLabel->setFontRole(dtk::FontRole::Mono);
            p.durationLabel->setMarginRole(dtk::SizeRole::MarginInside);
            p.durationLabel->setTooltip("Duration");

            p.timeUnitsComboBox = dtk::ComboBox::create(context);
            p.timeUnitsComboBox->setItems(timeline::getTimeUnitsLabels());
            p.timeUnitsComboBox->setCurrentIndex(
                static_cast<int>(timeUnitsModel->getTimeUnits()));
            p.timeUnitsComboBox->setTooltip("Time units");

            p.speedEdit = dtk::DoubleEdit::create(context, p.speedModel);
            p.speedEdit->setTooltip("Current speed");
            p.speedButton = dtk::ToolButton::create(context, "FPS");
            p.speedButton->setIcon("MenuArrow");
            p.speedButton->setTooltip("Speed menu");

            p.audioButton = dtk::ToolButton::create(context);
            p.audioButton->setIcon("Volume");
            p.audioButton->setTooltip("Audio volume");
            auto audioActions = p.audioActions->getActions();
            p.muteButton = dtk::ToolButton::create(context, audioActions["Mute"]);

            p.statusBar = StatusBar::create(context, app);
            p.statusBar->setHStretch(dtk::Stretch::Expanding);

            p.toolsWidget = ToolsWidget::create(
                context,
                app,
                std::dynamic_pointer_cast<MainWindow>(shared_from_this()));

            p.layout = dtk::VerticalLayout::create(context, shared_from_this());
            p.layout->setSpacingRole(dtk::SizeRole::None);
            p.menuBar->setParent(p.layout);
            p.dividers["MenuBar"] = dtk::Divider::create(context, dtk::Orientation::Vertical, p.layout);
            auto hLayout = dtk::HorizontalLayout::create(context, p.layout);
            hLayout->setSpacingRole(dtk::SizeRole::None);
            p.fileToolBar->setParent(hLayout);
            p.dividers["File"] = dtk::Divider::create(context, dtk::Orientation::Horizontal, hLayout);
            p.compareToolBar->setParent(hLayout);
            p.dividers["Compare"] = dtk::Divider::create(context, dtk::Orientation::Horizontal, hLayout);
            p.windowToolBar->setParent(hLayout);
            p.dividers["Window"] = dtk::Divider::create(context, dtk::Orientation::Horizontal, hLayout);
            p.viewToolBar->setParent(hLayout);
            p.dividers["View"] = dtk::Divider::create(context, dtk::Orientation::Horizontal, hLayout);
            p.toolsToolBar->setParent(hLayout);
            p.splitter = dtk::Splitter::create(context, dtk::Orientation::Vertical, p.layout);
            p.splitter->setSplit(settings.splitter);
            p.splitter2 = dtk::Splitter::create(context, dtk::Orientation::Horizontal, p.splitter);
            p.splitter2->setSplit(settings.splitter2);
            p.viewport->setParent(p.splitter2);
            p.toolsWidget->setParent(p.splitter2);
            p.timelineWidget->setParent(p.splitter);
            p.dividers["Bottom"] = dtk::Divider::create(context, dtk::Orientation::Vertical, p.layout);
            p.bottomLayout = dtk::HorizontalLayout::create(context, p.layout);
            p.bottomLayout->setMarginRole(dtk::SizeRole::MarginInside);
            p.bottomLayout->setSpacingRole(dtk::SizeRole::SpacingSmall);
            hLayout = dtk::HorizontalLayout::create(context, p.bottomLayout);
            hLayout->setSpacingRole(dtk::SizeRole::None);
            reverseButton->setParent(hLayout);
            stopButton->setParent(hLayout);
            forwardButton->setParent(hLayout);
            timeStartButton->setParent(hLayout);
            framePrevButton->setParent(hLayout);
            frameNextButton->setParent(hLayout);
            timeEndButton->setParent(hLayout);
            p.currentTimeEdit->setParent(p.bottomLayout);
            p.durationLabel->setParent(p.bottomLayout);
            p.timeUnitsComboBox->setParent(p.bottomLayout);
            hLayout = dtk::HorizontalLayout::create(context, p.bottomLayout);
            hLayout->setSpacingRole(dtk::SizeRole::SpacingTool);
            p.speedEdit->setParent(hLayout);
            p.speedButton->setParent(hLayout);
            auto spacer = dtk::Spacer::create(context, dtk::Orientation::Horizontal, p.bottomLayout);
            spacer->setHStretch(dtk::Stretch::Expanding);
            hLayout = dtk::HorizontalLayout::create(context, p.bottomLayout);
            hLayout->setSpacingRole(dtk::SizeRole::SpacingTool);
            p.audioButton->setParent(hLayout);
            p.muteButton->setParent(hLayout);
            p.dividers["Status"] = dtk::Divider::create(context, dtk::Orientation::Vertical, p.layout);
            p.statusBar->setParent(p.layout);

            auto appWeak = std::weak_ptr<App>(app);
            p.viewport->setCompareCallback(
                [appWeak](const timeline::CompareOptions& value)
                {
                    if (auto app = appWeak.lock())
                    {
                        app->getFilesModel()->setCompareOptions(value);
                    }
                });

            p.currentTimeEdit->setCallback(
                [this](const OTIO_NS::RationalTime& value)
                {
                    if (_p->player)
                    {
                        _p->player->setPlayback(timeline::Playback::Stop);
                        _p->player->seek(value);
                        _p->currentTimeEdit->setValue(_p->player->getCurrentTime());
                    }
                });

            p.timeUnitsComboBox->setIndexCallback(
                [appWeak](int value)
                {
                    if (auto app = appWeak.lock())
                    {
                        app->getTimeUnitsModel()->setTimeUnits(
                            static_cast<timeline::TimeUnits>(value));
                    }
                });

            p.speedButton->setPressedCallback(
                [this]
                {
                    _showSpeedPopup();
                });

            p.audioButton->setPressedCallback(
                [this]
                {
                    _showAudioPopup();
                });
            p.muteButton->setCheckedCallback(
                [appWeak](bool value)
                {
                    if (auto app = appWeak.lock())
                    {
                        app->getAudioModel()->setMute(value);
                    }
                });

            p.playerObserver = dtk::ValueObserver<std::shared_ptr<timeline::Player> >::create(
                app->observePlayer(),
                [this](const std::shared_ptr<timeline::Player>& value)
                {
                    _playerUpdate(value);
                });

            p.speedObserver2 = dtk::ValueObserver<double>::create(
                p.speedModel->observeValue(),
                [this](double value)
                {
                    if (_p->player)
                    {
                        _p->player->setSpeed(value);
                    }
                });

            p.ocioOptionsObserver = dtk::ValueObserver<timeline::OCIOOptions>::create(
                app->getColorModel()->observeOCIOOptions(),
                [this](const timeline::OCIOOptions& value)
                {
                    auto options = _p->timelineWidget->getDisplayOptions();
                    options.ocio = value;
                    _p->timelineWidget->setDisplayOptions(options);
                });

            p.lutOptionsObserver = dtk::ValueObserver<timeline::LUTOptions>::create(
                app->getColorModel()->observeLUTOptions(),
                [this](const timeline::LUTOptions& value)
                {
                    auto options = _p->timelineWidget->getDisplayOptions();
                    options.lut = value;
                    _p->timelineWidget->setDisplayOptions(options);
                });

            p.colorBufferObserver = dtk::ValueObserver<dtk::ImageType>::create(
                app->getViewportModel()->observeColorBuffer(),
                [this](dtk::ImageType value)
                {
                    setFrameBufferType(value);
                });

            p.muteObserver = dtk::ValueObserver<bool>::create(
                app->getAudioModel()->observeMute(),
                [this](bool value)
                {
                    _p->muteButton->setChecked(value);
                });

            p.windowSettingsObserver = dtk::ValueObserver<WindowSettings>::create(
                p.settingsModel->observeWindow(),
                [this](const WindowSettings& value)
                {
                    _settingsUpdate(value);
                });

            p.timelineSettingsObserver = dtk::ValueObserver<TimelineSettings>::create(
                p.settingsModel->observeTimeline(),
                [this](const TimelineSettings& value)
                {
                    _settingsUpdate(value);
                });
        }

        MainWindow::MainWindow() :
            _p(new Private)
        {}

        MainWindow::~MainWindow()
        {
            DTK_P();
            _makeCurrent();
            p.viewport->setParent(nullptr);
            p.timelineWidget->setParent(nullptr);

            WindowSettings settings = p.settingsModel->getWindow();
            settings.size = getGeometry().size();
#if defined(__APPLE__)
            //! \bug The window size needs to be scaled on macOS?
            const float displayScale = getDisplayScale();
            if (displayScale > 0.F)
            {
                settings.size = settings.size / displayScale;
            }
#endif // __APPLE__
            settings.splitter = p.splitter->getSplit();
            settings.splitter2 = p.splitter2->getSplit();
            p.settingsModel->setWindow(settings);
        }

        std::shared_ptr<MainWindow> MainWindow::create(
            const std::shared_ptr<dtk::Context>& context,
            const std::shared_ptr<App>& app)
        {
            auto out = std::shared_ptr<MainWindow>(new MainWindow);
            out->_init(context, app);
            return out;
        }

        const std::shared_ptr<Viewport>& MainWindow::getViewport() const
        {
            return _p->viewport;
        }

        const std::shared_ptr<timelineui::TimelineWidget>& MainWindow::getTimelineWidget() const
        {
            return _p->timelineWidget;
        }

        void MainWindow::focusCurrentFrame()
        {
            _p->currentTimeEdit->takeKeyFocus();
        }

        void MainWindow::setGeometry(const dtk::Box2I& value)
        {
            Window::setGeometry(value);
            _p->layout->setGeometry(value);
        }

        void MainWindow::keyPressEvent(dtk::KeyEvent& event)
        {
            DTK_P();
            event.accept = p.menuBar->shortcut(event.key, event.modifiers);
        }

        void MainWindow::keyReleaseEvent(dtk::KeyEvent& event)
        {
            event.accept = true;
        }

        void MainWindow::_drop(const std::vector<std::string>& value)
        {
            DTK_P();
            if (auto app = p.app.lock())
            {
                for (const auto& i : value)
                {
                    app->open(file::Path(i));
                }
            }
        }

        void MainWindow::_playerUpdate(const std::shared_ptr<timeline::Player>& value)
        {
            DTK_P();

            p.speedObserver.reset();
            p.currentTimeObserver.reset();

            p.player = value;

            p.viewport->setPlayer(p.player);
            p.timelineWidget->setPlayer(p.player);
            p.durationLabel->setValue(
                p.player ?
                p.player->getTimeRange().duration() :
                time::invalidTime);

            if (p.player)
            {
                p.speedObserver = dtk::ValueObserver<double>::create(
                    p.player->observeSpeed(),
                    [this](double value)
                    {
                        _p->speedModel->setValue(value);
                    });

                p.currentTimeObserver = dtk::ValueObserver<OTIO_NS::RationalTime>::create(
                    p.player->observeCurrentTime(),
                    [this](const OTIO_NS::RationalTime& value)
                    {
                        _p->currentTimeEdit->setValue(value);
                    });
            }
            else
            {
                p.speedModel->setValue(0.0);
                p.currentTimeEdit->setValue(time::invalidTime);
            }
        }

        void MainWindow::_showSpeedPopup()
        {
            DTK_P();
            if (auto context = getContext())
            {
                if (auto window = std::dynamic_pointer_cast<IWindow>(shared_from_this()))
                {
                    if (!p.speedPopup)
                    {
                        const double defaultSpeed =
                            p.player ?
                            p.player->getDefaultSpeed() :
                            0.0;
                        p.speedPopup = SpeedPopup::create(context, defaultSpeed);
                        p.speedPopup->open(window, p.speedButton->getGeometry());
                        auto weak = std::weak_ptr<MainWindow>(std::dynamic_pointer_cast<MainWindow>(shared_from_this()));
                        p.speedPopup->setCallback(
                            [weak](double value)
                            {
                                if (auto widget = weak.lock())
                                {
                                    if (widget->_p->player)
                                    {
                                        widget->_p->player->setSpeed(value);
                                    }
                                    widget->_p->speedPopup->close();
                                }
                            });
                        p.speedPopup->setCloseCallback(
                            [weak]
                            {
                                if (auto widget = weak.lock())
                                {
                                    widget->_p->speedPopup.reset();
                                }
                            });
                    }
                    else
                    {
                        p.speedPopup->close();
                        p.speedPopup.reset();
                    }
                }
            }
        }

        void MainWindow::_showAudioPopup()
        {
            DTK_P();
            if (auto context = getContext())
            {
                if (auto app = p.app.lock())
                {
                    if (auto window = std::dynamic_pointer_cast<IWindow>(shared_from_this()))
                    {
                        if (!p.audioPopup)
                        {
                            p.audioPopup = AudioPopup::create(context, app);
                            p.audioPopup->open(window, p.audioButton->getGeometry());
                            auto weak = std::weak_ptr<MainWindow>(std::dynamic_pointer_cast<MainWindow>(shared_from_this()));
                            p.audioPopup->setCloseCallback(
                                [weak]
                                {
                                    if (auto widget = weak.lock())
                                    {
                                        widget->_p->audioPopup.reset();
                                    }
                                });
                        }
                        else
                        {
                            p.audioPopup->close();
                            p.audioPopup.reset();
                        }
                    }
                }
            }
        }

        void MainWindow::_settingsUpdate(const WindowSettings& settings)
        {
            DTK_P();

            p.fileToolBar->setVisible(settings.fileToolBar);
            p.dividers["File"]->setVisible(settings.fileToolBar);

            p.compareToolBar->setVisible(settings.compareToolBar);
            p.dividers["Compare"]->setVisible(settings.compareToolBar);

            p.windowToolBar->setVisible(settings.windowToolBar);
            p.dividers["Window"]->setVisible(settings.windowToolBar);

            p.viewToolBar->setVisible(settings.viewToolBar);
            p.dividers["View"]->setVisible(settings.viewToolBar);

            p.toolsToolBar->setVisible(settings.toolsToolBar);

            p.timelineWidget->setVisible(settings.timeline);

            p.bottomLayout->setVisible(settings.bottomToolBar);
            p.dividers["Bottom"]->setVisible(settings.bottomToolBar);

            p.statusBar->setVisible(settings.statusToolBar);
            p.dividers["Status"]->setVisible(settings.statusToolBar);

            p.splitter->setSplit(settings.splitter);
            p.splitter2->setSplit(settings.splitter2);
        }

        void MainWindow::_settingsUpdate(const TimelineSettings& settings)
        {
            DTK_P();
            p.timelineWidget->setEditable(settings.editable);
            p.timelineWidget->setFrameView(settings.frameView);
            p.timelineWidget->setScrollToCurrentFrame(settings.scroll);
            p.timelineWidget->setStopOnScrub(settings.stopOnScrub);
            p.timelineWidget->setItemOptions(settings.item);
            timelineui::DisplayOptions display = settings.display;
            if (settings.firstTrack)
            {
                display.tracks = { 0 };
            }
            display.waveformHeight = display.thumbnailHeight / 2;
            p.timelineWidget->setDisplayOptions(display);
        }
    }
}
