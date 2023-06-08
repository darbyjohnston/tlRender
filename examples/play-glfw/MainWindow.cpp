// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2023 Darby Johnston
// All rights reserved.

#include "MainWindow.h"

#include <tlTimelineUI/TimelineViewport.h>
#include <tlTimelineUI/TimelineWidget.h>

#include <tlUI/ButtonGroup.h>
#include <tlUI/ComboBox.h>
#include <tlUI/Divider.h>
#include <tlUI/DoubleEdit.h>
#include <tlUI/DoubleModel.h>
#include <tlUI/IncButtons.h>
#include <tlUI/Label.h>
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
    namespace examples
    {
        namespace play_glfw
        {
            struct MainWindow::Private
            {
                std::shared_ptr<timeline::Player> player;
                std::shared_ptr<timeline::TimeUnitsModel> timeUnitsModel;
                std::shared_ptr<ui::DoubleModel> speedModel;
                timelineui::ItemOptions itemOptions;

                std::shared_ptr<ui::MenuBar> menuBar;
                std::shared_ptr<timelineui::TimelineViewport> timelineViewport;
                std::shared_ptr<timelineui::TimelineWidget> timelineWidget;
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

                std::shared_ptr<observer::ValueObserver<timeline::TimeUnits> > timeUnitsObserver;
                std::shared_ptr<observer::ValueObserver<double> > speedObserver;
                std::shared_ptr<observer::ValueObserver<double> > speedObserver2;
                std::shared_ptr<observer::ValueObserver<timeline::Playback> > playbackObserver;
                std::shared_ptr<observer::ValueObserver<otime::RationalTime> > currentTimeObserver;
            };

            void MainWindow::_init(
                const std::shared_ptr<timeline::Player>& player,
                const std::shared_ptr<system::Context>& context)
            {
                IWidget::_init("MainWindow", context);
                TLRENDER_P();

                setBackgroundRole(ui::ColorRole::Window);

                p.player = player;
                p.timeUnitsModel = timeline::TimeUnitsModel::create(context);
                p.speedModel = ui::DoubleModel::create(context);
                p.speedModel->setRange(math::DoubleRange(0.0, 1000.0));
                p.speedModel->setStep(1.F);
                p.speedModel->setLargeStep(10.F);

                auto fileMenuItem = ui::MenuItem::create("File");
                auto fileOpenMenuItem = ui::MenuItem::create("Open", fileMenuItem);
                auto fileCloseMenuItem = ui::MenuItem::create("Close", fileMenuItem);
                auto fileExitMenuItem = ui::MenuItem::create("Exit", fileMenuItem);
                auto compareMenuItem = ui::MenuItem::create("Compare");
                auto viewMenuItem = ui::MenuItem::create("View");
                auto renderMenuItem = ui::MenuItem::create("Render");
                auto playbackMenuItem = ui::MenuItem::create("Playback");
                auto audioMenuItem = ui::MenuItem::create("Audio");
                auto windowMenuItem = ui::MenuItem::create("Window");
                auto windowFullScreenMenuItem = ui::MenuItem::create("Full Screen", windowMenuItem);
                p.menuBar = ui::MenuBar::create(context);
                p.menuBar->addMenu(fileMenuItem);
                p.menuBar->addMenu(compareMenuItem);
                p.menuBar->addMenu(viewMenuItem);
                p.menuBar->addMenu(renderMenuItem);
                p.menuBar->addMenu(playbackMenuItem);
                p.menuBar->addMenu(audioMenuItem);
                p.menuBar->addMenu(windowMenuItem);

                p.timelineViewport = timelineui::TimelineViewport::create(context);
                p.timelineViewport->setPlayers({ player });

                p.timelineWidget = timelineui::TimelineWidget::create(context);
                p.timelineWidget->setScrollBarsVisible(false);
                p.timelineWidget->setPlayer(player);

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
                p.durationLabel->setValue(player->getTimeRange().duration());

                p.timeUnitsComboBox = ui::ComboBox::create(context);
                p.timeUnitsComboBox->setItems(timeline::getTimeUnitsLabels());
                p.timeUnitsComboBox->setCurrentIndex(
                    static_cast<int>(p.timeUnitsModel->getTimeUnits()));

                p.audioButton = ui::ToolButton::create(context);
                p.audioButton->setIcon("Volume");

                p.statusLabel = ui::Label::create(context);
                p.statusLabel->setTextWidth(20);
                p.statusLabel->setHStretch(ui::Stretch::Expanding);
                p.infoLabel = ui::Label::create(context);
                p.infoLabel->setTextWidth(20);

                p.layout = ui::VerticalLayout::create(context, shared_from_this());
                p.layout->setSpacingRole(ui::SizeRole::None);
                p.menuBar->setParent(p.layout);
                p.splitter = ui::Splitter::create(ui::Orientation::Vertical, context, p.layout);
                p.splitter->setSplit(.7F);
                p.timelineViewport->setParent(p.splitter);
                p.timelineWidget->setParent(p.splitter);
                ui::Divider::create(ui::Orientation::Vertical, context, p.layout);
                auto hLayout = ui::HorizontalLayout::create(context, p.layout);
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
                    [player](const otime::RationalTime& value)
                    {
                        player->seek(value);
                    });

                currentTimeIncButtons->setIncCallback(
                    [player]
                    {
                        player->frameNext();
                    });
                currentTimeIncButtons->setDecCallback(
                    [player]
                    {
                        player->framePrev();
                    });

                p.timeUnitsComboBox->setIndexCallback(
                    [this](int value)
                    {
                        _p->timeUnitsModel->setTimeUnits(
                            static_cast<timeline::TimeUnits>(value));
                    });

                p.timeUnitsObserver = observer::ValueObserver<timeline::TimeUnits>::create(
                    p.timeUnitsModel->observeTimeUnits(),
                    [this](timeline::TimeUnits value)
                    {
                        _p->itemOptions.timeUnits = value;
                        _p->timelineWidget->setItemOptions(_p->itemOptions);
                    });

                p.speedObserver = observer::ValueObserver<double>::create(
                    player->observeSpeed(),
                    [this](double value)
                    {
                        _p->speedModel->setValue(value);
                    });
                p.speedObserver2 = observer::ValueObserver<double>::create(
                    p.speedModel->observeValue(),
                    [player](double value)
                    {
                        player->setSpeed(value);
                    });

                p.playbackObserver = observer::ValueObserver<timeline::Playback>::create(
                    player->observePlayback(),
                    [this](timeline::Playback value)
                    {
                        _p->playbackButtonGroup->setChecked(static_cast<int>(value), true);
                    });

                p.currentTimeObserver = observer::ValueObserver<otime::RationalTime>::create(
                    player->observeCurrentTime(),
                    [this](const otime::RationalTime& value)
                    {
                        _p->currentTimeEdit->setValue(value);
                    });

                p.playbackButtonGroup->setCheckedCallback(
                    [player](int index, bool value)
                    {
                        player->setPlayback(static_cast<timeline::Playback>(index));
                    });

                p.frameButtonGroup->setClickedCallback(
                    [player](int index)
                    {
                        switch (index)
                        {
                        case 0:
                            player->timeAction(timeline::TimeAction::Start);
                            break;
                        case 1:
                            player->timeAction(timeline::TimeAction::FramePrev);
                            break;
                        case 2:
                            player->timeAction(timeline::TimeAction::FrameNext);
                            break;
                        case 3:
                            player->timeAction(timeline::TimeAction::End);
                            break;
                        }
                    });
            }

            MainWindow::MainWindow() :
                _p(new Private)
            {}

            MainWindow::~MainWindow()
            {}

            std::shared_ptr<MainWindow> MainWindow::create(
                const std::shared_ptr<timeline::Player>& player,
                const std::shared_ptr<system::Context>& context)
            {
                auto out = std::shared_ptr<MainWindow>(new MainWindow);
                out->_init(player, context);
                return out;
            }

            void MainWindow::setGeometry(const math::BBox2i& value)
            {
                IWidget::setGeometry(value);
                _p->layout->setGeometry(value);
            }

            void MainWindow::_infoUpdate()
            {
                TLRENDER_P();
                const file::Path& path = p.player->getPath();
                const io::Info& info = p.player->getIOInfo();
                const std::string text = string::Format("{0}").arg(path.get(-1, false));
                p.infoLabel->setText(text);
            }
        }
    }
}
