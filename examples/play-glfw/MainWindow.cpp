// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2023 Darby Johnston
// All rights reserved.

#include "MainWindow.h"

#include <tlUI/ButtonGroup.h>
#include <tlUI/RowLayout.h>
#include <tlUI/Splitter.h>
#include <tlUI/TimelineViewport.h>
#include <tlUI/TimelineWidget.h>
#include <tlUI/ToolButton.h>

namespace tl
{
    namespace examples
    {
        namespace play_glfw
        {
            struct MainWindow::Private
            {
                std::shared_ptr<ui::TimelineViewport> timelineViewport;
                std::shared_ptr<ui::TimelineWidget> timelineWidget;
                std::shared_ptr<ui::ButtonGroup> playbackButtonGroup;
                std::shared_ptr<ui::Splitter> splitter;
                std::shared_ptr<ui::RowLayout> layout;
                std::shared_ptr<observer::ValueObserver<timeline::Playback> > playbackObserver;
            };

            void MainWindow::_init(
                const std::shared_ptr<timeline::TimelinePlayer>& timelinePlayer,
                const std::shared_ptr<system::Context>& context)
            {
                IWidget::_init("MainWindow", context);
                TLRENDER_P();

                setBackgroundRole(ui::ColorRole::Window);

                p.timelineViewport = ui::TimelineViewport::create(context);
                p.timelineViewport->setTimelinePlayers({ timelinePlayer });

                p.timelineWidget = ui::TimelineWidget::create(context);
                p.timelineWidget->setTimelinePlayer(timelinePlayer);

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

                p.layout = ui::VerticalLayout::create(context, shared_from_this());
                p.layout->setSpacingRole(ui::SizeRole::None);
                p.splitter = ui::Splitter::create(ui::Orientation::Vertical, context, p.layout);
                p.splitter->setSplit(.7F);
                p.timelineViewport->setParent(p.splitter);
                p.timelineWidget->setParent(p.splitter);
                auto hLayout = ui::HorizontalLayout::create(context, p.layout);
                hLayout->setMarginRole(ui::SizeRole::MarginSmall);
                hLayout->setSpacingRole(ui::SizeRole::SpacingTool);
                reverseButton->setParent(hLayout);
                stopButton->setParent(hLayout);
                forwardButton->setParent(hLayout);

                p.playbackObserver = observer::ValueObserver<timeline::Playback>::create(
                    timelinePlayer->observePlayback(),
                    [this](timeline::Playback value)
                    {
                        _p->playbackButtonGroup->setChecked(static_cast<int>(value), true);
                    });

                p.playbackButtonGroup->setCheckedCallback(
                    [timelinePlayer](int index, bool value)
                    {
                        timelinePlayer->setPlayback(static_cast<timeline::Playback>(index));
                    });
            }

            MainWindow::MainWindow() :
                _p(new Private)
            {}

            MainWindow::~MainWindow()
            {}

            std::shared_ptr<MainWindow> MainWindow::create(
                const std::shared_ptr<timeline::TimelinePlayer>& timelinePlayer,
                const std::shared_ptr<system::Context>& context)
            {
                auto out = std::shared_ptr<MainWindow>(new MainWindow);
                out->_init(timelinePlayer, context);
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
