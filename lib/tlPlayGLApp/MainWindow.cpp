// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2023 Darby Johnston
// All rights reserved.

#include <tlPlayGLApp/MainWindow.h>

#include <tlPlayGLApp/App.h>
#include <tlPlayGLApp/AudioMenu.h>
#include <tlPlayGLApp/AudioPopup.h>
#include <tlPlayGLApp/CompareMenu.h>
#include <tlPlayGLApp/CompareToolBar.h>
#include <tlPlayGLApp/FileMenu.h>
#include <tlPlayGLApp/FileToolBar.h>
#include <tlPlayGLApp/FrameMenu.h>
#include <tlPlayGLApp/PlaybackMenu.h>
#include <tlPlayGLApp/RenderMenu.h>
#include <tlPlayGLApp/Settings.h>
#include <tlPlayGLApp/SpeedPopup.h>
#include <tlPlayGLApp/ToolsMenu.h>
#include <tlPlayGLApp/ToolsToolBar.h>
#include <tlPlayGLApp/ToolsWidget.h>
#include <tlPlayGLApp/ViewMenu.h>
#include <tlPlayGLApp/ViewToolBar.h>
#include <tlPlayGLApp/WindowMenu.h>
#include <tlPlayGLApp/WindowToolBar.h>

#include <tlTimelineUI/TimelineViewport.h>
#include <tlTimelineUI/TimelineWidget.h>

#include <tlUI/ButtonGroup.h>
#include <tlUI/ComboBox.h>
#include <tlUI/Divider.h>
#include <tlUI/DoubleEdit.h>
#include <tlUI/DoubleModel.h>
#include <tlUI/Label.h>
#include <tlUI/Menu.h>
#include <tlUI/MenuBar.h>
#include <tlUI/RowLayout.h>
#include <tlUI/Splitter.h>
#include <tlUI/TimeEdit.h>
#include <tlUI/TimeLabel.h>
#include <tlUI/ToolButton.h>

#include <tlTimeline/TimeUnits.h>

#include <tlCore/StringFormat.h>
#include <tlCore/Timer.h>

namespace tl
{
    namespace play_gl
    {
        struct MainWindow::Private
        {
            std::weak_ptr<App> app;
            std::weak_ptr<Settings> settings;
            std::shared_ptr<timeline::TimeUnitsModel> timeUnitsModel;
            std::shared_ptr<ui::DoubleModel> speedModel;
            timelineui::ItemOptions itemOptions;
            std::vector<std::shared_ptr<timeline::Player> > players;

            std::shared_ptr<timelineui::TimelineViewport> timelineViewport;
            std::shared_ptr<timelineui::TimelineWidget> timelineWidget;
            std::shared_ptr<FileMenu> fileMenu;
            std::shared_ptr<CompareMenu> compareMenu;
            std::shared_ptr<WindowMenu> windowMenu;
            std::shared_ptr<ViewMenu> viewMenu;
            std::shared_ptr<RenderMenu> renderMenu;
            std::shared_ptr<PlaybackMenu> playbackMenu;
            std::shared_ptr<FrameMenu> frameMenu;
            std::shared_ptr<AudioMenu> audioMenu;
            std::shared_ptr<ToolsMenu> toolsMenu;
            std::shared_ptr<ui::MenuBar> menuBar;
            std::shared_ptr<FileToolBar> fileToolBar;
            std::shared_ptr<CompareToolBar> compareToolBar;
            std::shared_ptr<WindowToolBar> windowToolBar;
            std::shared_ptr<ViewToolBar> viewToolBar;
            std::shared_ptr<ToolsToolBar> toolsToolBar;
            std::shared_ptr<ui::ButtonGroup> playbackButtonGroup;
            std::shared_ptr<ui::ButtonGroup> frameButtonGroup;
            std::shared_ptr<ui::TimeEdit> currentTimeEdit;
            std::shared_ptr<ui::DoubleEdit> speedEdit;
            std::shared_ptr<ui::ToolButton> speedButton;
            std::shared_ptr<SpeedPopup> speedPopup;
            std::shared_ptr<ui::TimeLabel> durationLabel;
            std::shared_ptr<ui::ComboBox> timeUnitsComboBox;
            std::shared_ptr<ui::ToolButton> audioButton;
            std::shared_ptr<AudioPopup> audioPopup;
            std::shared_ptr<ui::Label> statusLabel;
            std::shared_ptr<time::Timer> statusTimer;
            std::shared_ptr<ui::Label> infoLabel;
            std::shared_ptr<ToolsWidget> toolsWidget;
            std::shared_ptr<ui::Splitter> splitter;
            std::shared_ptr<ui::Splitter> splitter2;
            std::shared_ptr<ui::RowLayout> layout;

            std::shared_ptr<observer::ListObserver<std::shared_ptr<timeline::Player> > > playersObserver;
            std::shared_ptr<observer::ValueObserver<double> > speedObserver;
            std::shared_ptr<observer::ValueObserver<double> > speedObserver2;
            std::shared_ptr<observer::ValueObserver<timeline::Playback> > playbackObserver;
            std::shared_ptr<observer::ValueObserver<otime::RationalTime> > currentTimeObserver;
            std::shared_ptr<observer::ValueObserver<timeline::CompareOptions> > compareOptionsObserver;
            std::shared_ptr<observer::ListObserver<log::Item> > logObserver;
        };

        void MainWindow::_init(
            const std::shared_ptr<App>& app,
            const std::shared_ptr<system::Context>& context,
            const std::shared_ptr<IWidget>& parent)
        {
            IWidget::_init("tl::play_gl::MainWindow", context, parent);
            TLRENDER_P();

            setBackgroundRole(ui::ColorRole::Window);

            auto settings = app->getSettings();
            float splitter = .7F;
            settings->setDefaultValue("Window/Splitter", splitter);
            float splitter2 = .8F;
            settings->setDefaultValue("Window/Splitter2", splitter2);
            bool frameView = true;
            settings->setDefaultValue("Timeline/FrameView", frameView);
            bool stopOnScrub = true;
            settings->setDefaultValue("Timeline/StopOnScrub", stopOnScrub);
            timelineui::ItemOptions itemOptions;
            settings->setDefaultValue("Timeline/Thumbnails",
                itemOptions.thumbnails);
            settings->setDefaultValue("Timeline/ThumbnailsSize",
                itemOptions.thumbnailHeight);
            settings->setDefaultValue("Timeline/Transitions",
                itemOptions.showTransitions);
            settings->setDefaultValue("Timeline/Markers",
                itemOptions.showMarkers);
            p.settings = settings;

            p.app = app;
            p.timeUnitsModel = timeline::TimeUnitsModel::create(context);
            p.speedModel = ui::DoubleModel::create(context);
            p.speedModel->setRange(math::DoubleRange(0.0, 1000000.0));
            p.speedModel->setStep(1.F);
            p.speedModel->setLargeStep(10.F);

            p.timelineViewport = timelineui::TimelineViewport::create(context);

            p.timelineWidget = timelineui::TimelineWidget::create(p.timeUnitsModel, context);
            p.timelineWidget->setScrollBarsVisible(false);
            settings->getValue("Timeline/FrameView", frameView);
            p.timelineWidget->setFrameView(frameView);
            settings->getValue("Timeline/FrameView", stopOnScrub);
            p.timelineWidget->setStopOnScrub(stopOnScrub);
            settings->getValue("Timeline/Thumbnails", itemOptions.thumbnails);
            settings->getValue("Timeline/ThumbnailsSize", itemOptions.thumbnailHeight);
            settings->getValue("Timeline/Transitions", itemOptions.showTransitions);
            settings->getValue("Timeline/Markers", itemOptions.showMarkers);
            p.timelineWidget->setItemOptions(itemOptions);

            p.fileMenu = FileMenu::create(app, context);
            p.compareMenu = CompareMenu::create(app, context);
            p.windowMenu = WindowMenu::create(app, context);
            p.viewMenu = ViewMenu::create(
                std::dynamic_pointer_cast<MainWindow>(shared_from_this()),
                app,
                context);
            p.renderMenu = RenderMenu::create(app, context);
            p.playbackMenu = PlaybackMenu::create(
                std::dynamic_pointer_cast<MainWindow>(shared_from_this()),
                app,
                context);
            p.frameMenu = FrameMenu::create(
                std::dynamic_pointer_cast<MainWindow>(shared_from_this()),
                app,
                context);
            p.audioMenu = AudioMenu::create(app, context);
            p.toolsMenu = ToolsMenu::create(app, context);
            p.menuBar = ui::MenuBar::create(context);
            p.menuBar->addMenu("File", p.fileMenu);
            p.menuBar->addMenu("Compare", p.compareMenu);
            p.menuBar->addMenu("Window", p.windowMenu);
            p.menuBar->addMenu("View", p.viewMenu);
            p.menuBar->addMenu("Render", p.renderMenu);
            p.menuBar->addMenu("Playback", p.playbackMenu);
            p.menuBar->addMenu("Frame", p.frameMenu);
            p.menuBar->addMenu("Audio", p.audioMenu);
            p.menuBar->addMenu("Tools", p.toolsMenu);

            p.fileToolBar = FileToolBar::create(app, context);
            p.compareToolBar = CompareToolBar::create(app, context);
            p.windowToolBar = WindowToolBar::create(app, context);
            p.viewToolBar = ViewToolBar::create(
                std::dynamic_pointer_cast<MainWindow>(shared_from_this()),
                app,
                context);
            p.toolsToolBar = ToolsToolBar::create(app, context);

            auto stopButton = ui::ToolButton::create(context);
            stopButton->setIcon("PlaybackStop");
            auto forwardButton = ui::ToolButton::create(context);
            forwardButton->setIcon("PlaybackForward");
            auto reverseButton = ui::ToolButton::create(context);
            reverseButton->setIcon("PlaybackReverse");
            p.playbackButtonGroup = ui::ButtonGroup::create(ui::ButtonGroupType::Radio, context);
            p.playbackButtonGroup->addButton(stopButton);
            p.playbackButtonGroup->addButton(forwardButton);
            p.playbackButtonGroup->addButton(reverseButton);

            auto timeStartButton = ui::ToolButton::create(context);
            timeStartButton->setIcon("TimeStart");
            auto timeEndButton = ui::ToolButton::create(context);
            timeEndButton->setIcon("TimeEnd");
            auto framePrevButton = ui::ToolButton::create(context);
            framePrevButton->setIcon("FramePrev");
            framePrevButton->setRepeatClick(true);
            auto frameNextButton = ui::ToolButton::create(context);
            frameNextButton->setIcon("FrameNext");
            frameNextButton->setRepeatClick(true);
            p.frameButtonGroup = ui::ButtonGroup::create(ui::ButtonGroupType::Click, context);
            p.frameButtonGroup->addButton(timeStartButton);
            p.frameButtonGroup->addButton(framePrevButton);
            p.frameButtonGroup->addButton(frameNextButton);
            p.frameButtonGroup->addButton(timeEndButton);

            p.currentTimeEdit = ui::TimeEdit::create(p.timeUnitsModel, context);

            p.speedEdit = ui::DoubleEdit::create(context, p.speedModel);
            p.speedButton = ui::ToolButton::create(context);
            p.speedButton->setIcon("MenuArrow");

            p.durationLabel = ui::TimeLabel::create(p.timeUnitsModel, context);
            p.durationLabel->setMarginRole(ui::SizeRole::MarginInside);

            p.timeUnitsComboBox = ui::ComboBox::create(context);
            p.timeUnitsComboBox->setItems(timeline::getTimeUnitsLabels());
            p.timeUnitsComboBox->setCurrentIndex(
                static_cast<int>(p.timeUnitsModel->getTimeUnits()));

            p.audioButton = ui::ToolButton::create(context);
            p.audioButton->setIcon("Volume");

            p.statusLabel = ui::Label::create(context);
            p.statusLabel->setHStretch(ui::Stretch::Expanding);
            p.statusLabel->setMarginRole(ui::SizeRole::MarginInside);
            p.statusTimer = time::Timer::create(context);

            p.infoLabel = ui::Label::create(context);
            p.infoLabel->setHAlign(ui::HAlign::Right);
            p.infoLabel->setMarginRole(ui::SizeRole::MarginInside);

            p.toolsWidget = ToolsWidget::create(app, context);
            p.toolsWidget->setVisible(false);

            p.layout = ui::VerticalLayout::create(context, shared_from_this());
            p.layout->setSpacingRole(ui::SizeRole::None);
            p.menuBar->setParent(p.layout);
            ui::Divider::create(ui::Orientation::Vertical, context, p.layout);
            auto hLayout = ui::HorizontalLayout::create(context, p.layout);
            hLayout->setSpacingRole(ui::SizeRole::None);
            p.fileToolBar->setParent(hLayout);
            ui::Divider::create(ui::Orientation::Horizontal, context, hLayout);
            p.compareToolBar->setParent(hLayout);
            ui::Divider::create(ui::Orientation::Horizontal, context, hLayout);
            p.windowToolBar->setParent(hLayout);
            ui::Divider::create(ui::Orientation::Horizontal, context, hLayout);
            p.viewToolBar->setParent(hLayout);
            ui::Divider::create(ui::Orientation::Horizontal, context, hLayout);
            p.toolsToolBar->setParent(hLayout);
            ui::Divider::create(ui::Orientation::Vertical, context, p.layout);
            p.splitter = ui::Splitter::create(ui::Orientation::Vertical, context, p.layout);
            settings->getValue("Window/Splitter", splitter);
            p.splitter->setSplit(splitter);
            p.splitter->setSpacingRole(ui::SizeRole::None);
            p.splitter2 = ui::Splitter::create(ui::Orientation::Horizontal, context, p.splitter);
            settings->getValue("Window/Splitter2", splitter2);
            p.splitter2->setSplit(splitter2);
            p.splitter2->setSpacingRole(ui::SizeRole::None);
            p.timelineViewport->setParent(p.splitter2);
            p.toolsWidget->setParent(p.splitter2);
            p.timelineWidget->setParent(p.splitter);
            ui::Divider::create(ui::Orientation::Vertical, context, p.layout);
            hLayout = ui::HorizontalLayout::create(context, p.layout);
            hLayout->setMarginRole(ui::SizeRole::MarginInside);
            hLayout->setSpacingRole(ui::SizeRole::SpacingSmall);
            auto hLayout2 = ui::HorizontalLayout::create(context, hLayout);
            hLayout2->setSpacingRole(ui::SizeRole::None);
            reverseButton->setParent(hLayout2);
            stopButton->setParent(hLayout2);
            forwardButton->setParent(hLayout2);
            timeStartButton->setParent(hLayout2);
            framePrevButton->setParent(hLayout2);
            frameNextButton->setParent(hLayout2);
            timeEndButton->setParent(hLayout2);
            p.currentTimeEdit->setParent(hLayout);
            hLayout2 = ui::HorizontalLayout::create(context, hLayout);
            hLayout2->setSpacingRole(ui::SizeRole::SpacingTool);
            p.speedEdit->setParent(hLayout2);
            p.speedButton->setParent(hLayout2);
            p.durationLabel->setParent(hLayout);
            p.timeUnitsComboBox->setParent(hLayout);
            p.audioButton->setParent(hLayout);
            ui::Divider::create(ui::Orientation::Vertical, context, p.layout);
            hLayout = ui::HorizontalLayout::create(context, p.layout);
            hLayout->setSpacingRole(ui::SizeRole::None);
            p.statusLabel->setParent(hLayout);
            ui::Divider::create(ui::Orientation::Horizontal, context, hLayout);
            p.infoLabel->setParent(hLayout);

            _viewportUpdate();
            _infoUpdate();

            p.currentTimeEdit->setCallback(
                [this](const otime::RationalTime& value)
                {
                    if (!_p->players.empty() && _p->players[0])
                    {
                        _p->players[0]->seek(value);
                    }
                });

            p.timeUnitsComboBox->setIndexCallback(
                [this](int value)
                {
                    _p->timeUnitsModel->setTimeUnits(
                        static_cast<timeline::TimeUnits>(value));
                });

            p.playbackButtonGroup->setCheckedCallback(
                [this](int index, bool value)
                {
                    if (!_p->players.empty() && _p->players[0])
                    {
                        _p->players[0]->setPlayback(static_cast<timeline::Playback>(index));
                    }
                });

            p.frameButtonGroup->setClickedCallback(
                [this](int index)
                {
                    if (!_p->players.empty() && _p->players[0])
                    {
                        switch (index)
                        {
                        case 0:
                            _p->players[0]->timeAction(timeline::TimeAction::Start);
                            break;
                        case 1:
                            _p->players[0]->timeAction(timeline::TimeAction::FramePrev);
                            break;
                        case 2:
                            _p->players[0]->timeAction(timeline::TimeAction::FrameNext);
                            break;
                        case 3:
                            _p->players[0]->timeAction(timeline::TimeAction::End);
                            break;
                        }
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

            p.playersObserver = observer::ListObserver<std::shared_ptr<timeline::Player> >::create(
                app->observeActivePlayers(),
                [this](const std::vector<std::shared_ptr<timeline::Player> >& value)
                {
                    _setPlayers(value);
                });

            p.speedObserver2 = observer::ValueObserver<double>::create(
                p.speedModel->observeValue(),
                [this](double value)
                {
                    if (!_p->players.empty() && _p->players[0])
                    {
                        _p->players[0]->setSpeed(value);
                    }
                });

            p.compareOptionsObserver = observer::ValueObserver<timeline::CompareOptions>::create(
                app->getFilesModel()->observeCompareOptions(),
                [this](const timeline::CompareOptions& value)
                {
                    _viewportUpdate();
                });

            p.logObserver = observer::ListObserver<log::Item>::create(
                context->getLogSystem()->observeLog(),
                [this](const std::vector<log::Item>& value)
                {
                    _statusUpdate(value);
                });
        }

        MainWindow::MainWindow() :
            _p(new Private)
        {}

        MainWindow::~MainWindow()
        {
            TLRENDER_P();
            if (auto settings = p.settings.lock())
            {
                float splitter = p.splitter->getSplit();
                settings->setValue("Window/Splitter", splitter);
                settings->setValue("Window/Splitter2", p.splitter2->getSplit());
                settings->setValue("Timeline/FrameView",
                    p.timelineWidget->hasFrameView());
                settings->setValue("Timeline/StopOnScrub",
                    p.timelineWidget->hasStopOnScrub());
                const auto& timelineItemOptions = p.timelineWidget->getItemOptions();
                settings->setValue("Timeline/Thumbnails",
                    timelineItemOptions.thumbnails);
                settings->setValue("Timeline/ThumbnailsSize",
                    timelineItemOptions.thumbnailHeight);
                settings->setValue("Timeline/Transitions",
                    timelineItemOptions.showTransitions);
                settings->setValue("Timeline/Markers",
                    timelineItemOptions.showMarkers);
            }
        }

        std::shared_ptr<MainWindow> MainWindow::create(
            const std::shared_ptr<App>& app,
            const std::shared_ptr<system::Context>& context,
            const std::shared_ptr<IWidget>& parent)
        {
            auto out = std::shared_ptr<MainWindow>(new MainWindow);
            out->_init(app, context, parent);
            return out;
        }

        const std::shared_ptr<timelineui::TimelineViewport>& MainWindow::getTimelineViewport() const
        {
            return _p->timelineViewport;
        }

        const std::shared_ptr<timelineui::TimelineWidget>& MainWindow::getTimelineWidget() const
        {
            return _p->timelineWidget;
        }

        void MainWindow::focusCurrentFrame()
        {
            _p->currentTimeEdit->takeKeyFocus();
        }

        void MainWindow::setGeometry(const math::BBox2i& value)
        {
            IWidget::setGeometry(value);
            _p->layout->setGeometry(value);
        }

        void MainWindow::keyPressEvent(ui::KeyEvent& event)
        {
            TLRENDER_P();
            event.accept = p.menuBar->shortcut(event.key, event.modifiers);
        }

        void MainWindow::keyReleaseEvent(ui::KeyEvent& event)
        {
            event.accept = true;
        }

        void MainWindow::_setPlayers(const std::vector<std::shared_ptr<timeline::Player> >& value)
        {
            TLRENDER_P();

            p.speedObserver.reset();
            p.playbackObserver.reset();
            p.currentTimeObserver.reset();

            p.players = value;

            p.timelineViewport->setPlayers(p.players);
            p.timelineWidget->setPlayer(
                !p.players.empty() ?
                p.players[0] :
                nullptr);
            p.durationLabel->setValue(
                (!p.players.empty() && p.players[0]) ?
                p.players[0]->getTimeRange().duration() :
                time::invalidTime);
            _infoUpdate();

            if (!p.players.empty() && p.players[0])
            {
                p.speedObserver = observer::ValueObserver<double>::create(
                    p.players[0]->observeSpeed(),
                    [this](double value)
                    {
                        _p->speedModel->setValue(value);
                    });

                p.playbackObserver = observer::ValueObserver<timeline::Playback>::create(
                    p.players[0]->observePlayback(),
                    [this](timeline::Playback value)
                    {
                        _p->playbackButtonGroup->setChecked(static_cast<int>(value), true);
                    });

                p.currentTimeObserver = observer::ValueObserver<otime::RationalTime>::create(
                    p.players[0]->observeCurrentTime(),
                    [this](const otime::RationalTime& value)
                    {
                        _p->currentTimeEdit->setValue(value);
                    });
            }
            else
            {
                p.speedModel->setValue(0.0);
                p.playbackButtonGroup->setChecked(0, true);
                p.currentTimeEdit->setValue(time::invalidTime);
            }
        }

        void MainWindow::_showSpeedPopup()
        {
            TLRENDER_P();
            if (auto context = _context.lock())
            {
                if (auto eventLoop = getEventLoop().lock())
                {
                    if (!p.speedPopup)
                    {
                        const double defaultSpeed =
                            !p.players.empty() && p.players[0] ?
                            p.players[0]->getDefaultSpeed() :
                            0.0;
                        p.speedPopup = SpeedPopup::create(defaultSpeed, context);
                        p.speedPopup->open(eventLoop, p.speedButton->getGeometry());
                        auto weak = std::weak_ptr<MainWindow>(std::dynamic_pointer_cast<MainWindow>(shared_from_this()));
                        p.speedPopup->setCallback(
                            [weak](double value)
                            {
                                if (auto widget = weak.lock())
                                {
                                    if (!widget->_p->players.empty() &&
                                        widget->_p->players[0])
                                    {
                                        widget->_p->players[0]->setSpeed(value);
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
            TLRENDER_P();
            if (auto context = _context.lock())
            {
                if (auto app = p.app.lock())
                {
                    if (auto eventLoop = getEventLoop().lock())
                    {
                        if (!p.audioPopup)
                        {
                            p.audioPopup = AudioPopup::create(app, context);
                            p.audioPopup->open(eventLoop, p.audioButton->getGeometry());
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

        void MainWindow::_viewportUpdate()
        {
            TLRENDER_P();
            if (auto app = p.app.lock())
            {
                p.timelineViewport->setCompareOptions(
                    app->getFilesModel()->getCompareOptions());
            }
        }

        void MainWindow::_statusUpdate(const std::vector<log::Item>& value)
        {
            TLRENDER_P();
            for (const auto& i : value)
            {
                switch (i.type)
                {
                case log::Type::Error:
                    p.statusLabel->setText(log::toString(i));
                    p.statusTimer->start(
                        std::chrono::seconds(5),
                        [this]
                        {
                            _p->statusLabel->setText(std::string());
                        });
                        break;
                default: break;
                }
            }
        }

        void MainWindow::_infoUpdate()
        {
            TLRENDER_P();
            std::string text;
            if (!p.players.empty() && p.players[0])
            {
                const file::Path& path = p.players[0]->getPath();
                const io::Info& info = p.players[0]->getIOInfo();
                std::vector<std::string> s;
                s.push_back(path.get(-1, false));
                if (!info.video.empty())
                {
                    s.push_back(std::string(
                        string::Format("V: {0} {1}").
                        arg(info.video[0].size).
                        arg(info.video[0].pixelType)));
                }
                if (info.audio.isValid())
                {
                    s.push_back(std::string(
                        string::Format("A: {0} {1} {2}").
                        arg(info.audio.channelCount).
                        arg(info.audio.dataType).
                        arg(info.audio.sampleRate / 1000)));
                }
                text = string::join(s, ", ");
            }
            p.infoLabel->setText(text);
        }
    }
}
