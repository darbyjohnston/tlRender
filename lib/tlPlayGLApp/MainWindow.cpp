// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2023 Darby Johnston
// All rights reserved.

#include <tlPlayGLApp/MainWindow.h>

#include <tlPlayGLApp/App.h>
#include <tlPlayGLApp/AudioMenu.h>
#include <tlPlayGLApp/CompareMenu.h>
#include <tlPlayGLApp/CompareToolBar.h>
#include <tlPlayGLApp/FileMenu.h>
#include <tlPlayGLApp/FileToolBar.h>
#include <tlPlayGLApp/FrameMenu.h>
#include <tlPlayGLApp/PlaybackMenu.h>
#include <tlPlayGLApp/RenderMenu.h>
#include <tlPlayGLApp/ToolsMenu.h>
#include <tlPlayGLApp/ToolsToolBar.h>
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
#include <tlUI/IncButtons.h>
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

namespace tl
{
    namespace play_gl
    {
        struct MainWindow::Private
        {
            std::shared_ptr<timeline::Player> player;
            std::shared_ptr<timeline::TimeUnitsModel> timeUnitsModel;
            std::shared_ptr<ui::DoubleModel> speedModel;
            timelineui::ItemOptions itemOptions;

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
            std::shared_ptr<ui::TimeLabel> durationLabel;
            std::shared_ptr<ui::ComboBox> timeUnitsComboBox;
            std::shared_ptr<ui::ToolButton> audioButton;
            std::shared_ptr<ui::Label> statusLabel;
            std::shared_ptr<ui::Label> infoLabel;
            std::shared_ptr<ui::Splitter> splitter;
            std::shared_ptr<ui::RowLayout> layout;

            std::shared_ptr<observer::ValueObserver<std::shared_ptr<timeline::Player> > > playerObserver;
            std::shared_ptr<observer::ValueObserver<double> > speedObserver;
            std::shared_ptr<observer::ValueObserver<double> > speedObserver2;
            std::shared_ptr<observer::ValueObserver<timeline::Playback> > playbackObserver;
            std::shared_ptr<observer::ValueObserver<otime::RationalTime> > currentTimeObserver;
        };

        void MainWindow::_init(
            const std::shared_ptr<App>& app,
            const std::shared_ptr<system::Context>& context)
        {
            IWidget::_init("MainWindow", context);
            TLRENDER_P();

            setBackgroundRole(ui::ColorRole::Window);

            p.timeUnitsModel = timeline::TimeUnitsModel::create(context);
            p.speedModel = ui::DoubleModel::create(context);
            p.speedModel->setRange(math::DoubleRange(0.0, 1000000.0));
            p.speedModel->setStep(1.F);
            p.speedModel->setLargeStep(10.F);

            p.timelineViewport = timelineui::TimelineViewport::create(context);

            p.timelineWidget = timelineui::TimelineWidget::create(p.timeUnitsModel, context);
            p.timelineWidget->setScrollBarsVisible(false);

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
            auto currentTimeIncButtons = ui::IncButtons::create(context);

            p.speedEdit = ui::DoubleEdit::create(p.speedModel, context);
            auto speedIncButtons = ui::DoubleIncButtons::create(p.speedModel, context);
            p.speedButton = ui::ToolButton::create(context);
            p.speedButton->setIcon("MenuArrow");

            p.durationLabel = ui::TimeLabel::create(p.timeUnitsModel, context);

            p.timeUnitsComboBox = ui::ComboBox::create(context);
            p.timeUnitsComboBox->setItems(timeline::getTimeUnitsLabels());
            p.timeUnitsComboBox->setCurrentIndex(
                static_cast<int>(p.timeUnitsModel->getTimeUnits()));

            p.audioButton = ui::ToolButton::create(context);
            p.audioButton->setIcon("Volume");

            p.statusLabel = ui::Label::create(context);
            p.statusLabel->setTextWidth(80);
            p.statusLabel->setHStretch(ui::Stretch::Expanding);
            p.infoLabel = ui::Label::create(context);
            p.infoLabel->setTextWidth(40);

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
            p.splitter = ui::Splitter::create(ui::Orientation::Vertical, context, p.layout);
            p.splitter->setSplit(.7F);
            p.timelineViewport->setParent(p.splitter);
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
            hLayout2 = ui::HorizontalLayout::create(context, hLayout);
            hLayout2->setSpacingRole(ui::SizeRole::SpacingTool);
            p.currentTimeEdit->setParent(hLayout2);
            currentTimeIncButtons->setParent(hLayout2);
            hLayout2 = ui::HorizontalLayout::create(context, hLayout);
            hLayout2->setSpacingRole(ui::SizeRole::SpacingTool);
            p.speedEdit->setParent(hLayout2);
            speedIncButtons->setParent(hLayout2);
            p.speedButton->setParent(hLayout2);
            p.durationLabel->setParent(hLayout);
            p.timeUnitsComboBox->setParent(hLayout);
            p.audioButton->setParent(hLayout);
            ui::Divider::create(ui::Orientation::Vertical, context, p.layout);
            hLayout = ui::HorizontalLayout::create(context, p.layout);
            hLayout->setMarginRole(ui::SizeRole::MarginInside);
            hLayout->setSpacingRole(ui::SizeRole::SpacingSmall);
            p.statusLabel->setParent(hLayout);
            p.infoLabel->setParent(hLayout);

            _infoUpdate();

            p.currentTimeEdit->setCallback(
                [this](const otime::RationalTime& value)
                {
                    if (_p->player)
                    {
                        _p->player->seek(value);
                    }
                });

            currentTimeIncButtons->setIncCallback(
                [this]
                {
                    if (_p->player)
                    {
                        _p->player->frameNext();
                    }
                });
            currentTimeIncButtons->setDecCallback(
                [this]
                {
                    if (_p->player)
                    {
                        _p->player->framePrev();
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
                    if (_p->player)
                    {
                        _p->player->setPlayback(static_cast<timeline::Playback>(index));
                    }
                });

            p.frameButtonGroup->setClickedCallback(
                [this](int index)
                {
                    if (_p->player)
                    {
                        switch (index)
                        {
                        case 0:
                            _p->player->timeAction(timeline::TimeAction::Start);
                            break;
                        case 1:
                            _p->player->timeAction(timeline::TimeAction::FramePrev);
                            break;
                        case 2:
                            _p->player->timeAction(timeline::TimeAction::FrameNext);
                            break;
                        case 3:
                            _p->player->timeAction(timeline::TimeAction::End);
                            break;
                        }
                    }
                });

            p.playerObserver = observer::ValueObserver<std::shared_ptr<timeline::Player> >::create(
                app->observePlayer(),
                [this](const std::shared_ptr<timeline::Player>& value)
                {
                    _setPlayer(value);
                });

            p.speedObserver2 = observer::ValueObserver<double>::create(
                p.speedModel->observeValue(),
                [this](double value)
                {
                    if (_p->player)
                    {
                        _p->player->setSpeed(value);
                    }
                });
        }

        MainWindow::MainWindow() :
            _p(new Private)
        {}

        MainWindow::~MainWindow()
        {}

        std::shared_ptr<MainWindow> MainWindow::create(
            const std::shared_ptr<App>& app,
            const std::shared_ptr<system::Context>& context)
        {
            auto out = std::shared_ptr<MainWindow>(new MainWindow);
            out->_init(app, context);
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

        void MainWindow::_setPlayer(const std::shared_ptr<timeline::Player>& value)
        {
            TLRENDER_P();

            p.speedObserver.reset();
            p.playbackObserver.reset();
            p.currentTimeObserver.reset();

            p.player = value;

            if (p.player)
            {
                p.timelineViewport->setPlayers({ p.player });
            }
            else
            {
                p.timelineViewport->setPlayers({});
            }
            p.timelineWidget->setPlayer(p.player);
            p.durationLabel->setValue(p.player ?
                p.player->getTimeRange().duration() :
                time::invalidTime);
            _infoUpdate();

            if (p.player)
            {
                p.speedObserver = observer::ValueObserver<double>::create(
                    p.player->observeSpeed(),
                    [this](double value)
                    {
                        _p->speedModel->setValue(value);
                    });

                p.playbackObserver = observer::ValueObserver<timeline::Playback>::create(
                    p.player->observePlayback(),
                    [this](timeline::Playback value)
                    {
                        _p->playbackButtonGroup->setChecked(static_cast<int>(value), true);
                    });

                p.currentTimeObserver = observer::ValueObserver<otime::RationalTime>::create(
                    p.player->observeCurrentTime(),
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

        void MainWindow::_infoUpdate()
        {
            TLRENDER_P();
            std::string text;
            if (p.player)
            {
                const file::Path& path = p.player->getPath();
                const io::Info& info = p.player->getIOInfo();
                text = string::Format("{0}").arg(path.get(-1, false));
            }
            p.infoLabel->setText(text);
        }
    }
}
