// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2023 Darby Johnston
// All rights reserved.

#include "MainWindow.h"

#include <tlTimelineUI/TimelineViewport.h>
#include <tlTimelineUI/TimelineWidget.h>

#include <tlUI/ButtonGroup.h>
#include <tlUI/IncButtons.h>
#include <tlUI/Label.h>
#include <tlUI/LineEdit.h>
#include <tlUI/RowLayout.h>
#include <tlUI/Splitter.h>
#include <tlUI/ToolButton.h>

#include <tlCore/StringFormat.h>

namespace tl
{
    namespace examples
    {
        namespace play_glfw
        {
            struct MainWindow::Private
            {
                std::shared_ptr<timelineui::TimelineViewport> timelineViewport;
                std::shared_ptr<timelineui::TimelineWidget> timelineWidget;
                std::shared_ptr<ui::ButtonGroup> playbackButtonGroup;
                std::shared_ptr<ui::ButtonGroup> frameButtonGroup;
                std::shared_ptr<ui::LineEdit> currentTimeEdit;
                std::shared_ptr<ui::Label> durationLabel;
                std::shared_ptr<ui::LineEdit> speedEdit;
                std::shared_ptr<ui::Splitter> splitter;
                std::shared_ptr<ui::RowLayout> layout;

                std::shared_ptr<observer::ValueObserver<double> > speedObserver;
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

                p.timelineViewport = timelineui::TimelineViewport::create(context);
                p.timelineViewport->setPlayers({ player });

                p.timelineWidget = timelineui::TimelineWidget::create(context);
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

                p.currentTimeEdit = ui::LineEdit::create(context);
                p.currentTimeEdit->setFormat("00:00:00:00");
                auto currentTimeIncButtons = ui::IncButtons::create(context);

                p.durationLabel = ui::Label::create(context);
                p.durationLabel->setText(
                    player->getTimeRange().duration().to_timecode());

                p.speedEdit = ui::LineEdit::create(context);
                p.speedEdit->setFormat("00.00");
                auto speedIncButtons = ui::IncButtons::create(context);

                p.layout = ui::VerticalLayout::create(context, shared_from_this());
                p.layout->setSpacingRole(ui::SizeRole::None);
                p.splitter = ui::Splitter::create(ui::Orientation::Vertical, context, p.layout);
                p.splitter->setSplit(.7F);
                p.timelineViewport->setParent(p.splitter);
                p.timelineWidget->setParent(p.splitter);
                auto hLayout = ui::HorizontalLayout::create(context, p.layout);
                hLayout->setMarginRole(ui::SizeRole::MarginSmall);
                hLayout->setSpacingRole(ui::SizeRole::SpacingSmall);
                auto hLayout2 = ui::HorizontalLayout::create(context, hLayout);
                hLayout2->setSpacingRole(ui::SizeRole::SpacingTool);
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
                p.durationLabel->setParent(hLayout);
                hLayout2 = ui::HorizontalLayout::create(context, hLayout);
                hLayout2->setSpacingRole(ui::SizeRole::SpacingTool);
                p.speedEdit->setParent(hLayout2);
                speedIncButtons->setParent(hLayout2);

                p.speedObserver = observer::ValueObserver<double>::create(
                    player->observeSpeed(),
                    [this](double value)
                    {
                        const std::string s = string::Format("{0}").arg(value, 2, 2);
                        _p->speedEdit->setText(s);
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
                        const std::string s = value.to_timecode();
                        _p->currentTimeEdit->setText(s);
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
        }
    }
}
