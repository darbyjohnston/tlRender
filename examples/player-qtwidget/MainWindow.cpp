// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2024 Darby Johnston
// All rights reserved.

#include "MainWindow.h"

#include <QActionGroup>
#include <QDockWidget>
#include <QStyle>
#include <QToolBar>
#include <QToolButton>

namespace tl
{
    namespace examples
    {
        namespace player_qtwidget
        {
            MainWindow::MainWindow(const std::shared_ptr<dtk::Context>& context)
            {
                // Create the models.
                auto timeUnitsModel = timeline::TimeUnitsModel::create(context);
                auto timeObject = new qt::TimeObject(timeUnitsModel, this);
                auto style = dtk::Style::create(context);

                // Create the viewport.
                _viewport = new qtwidget::Viewport(context, style);
                setCentralWidget(_viewport);

                // Create the timeline widget.
                _timelineWidget = new qtwidget::TimelineWidget(context, timeUnitsModel, style);
                auto timelineDockWidget = new QDockWidget;
                timelineDockWidget->setFeatures(QDockWidget::NoDockWidgetFeatures);
                timelineDockWidget->setTitleBarWidget(new QWidget);
                timelineDockWidget->setWidget(_timelineWidget);
                addDockWidget(Qt::BottomDockWidgetArea, timelineDockWidget);

                // Create the tool bar.
                _stopAction = new QAction(this);
                _stopAction->setCheckable(true);
                _stopAction->setIcon(this->style()->standardIcon(QStyle::SP_MediaStop));
                _stopAction->setToolTip(tr("Stop playback"));
                _forwardAction = new QAction(this);
                _forwardAction->setCheckable(true);
                _forwardAction->setIcon(this->style()->standardIcon(QStyle::SP_MediaPlay));
                _forwardAction->setToolTip(tr("Forward playback"));
                _playbackActionGroup = new QActionGroup(this);
                _playbackActionGroup->setExclusive(true);
                _playbackActionGroup->addAction(_stopAction);
                _playbackActionGroup->addAction(_forwardAction);
                _currentTimeSpinBox = new qtwidget::TimeSpinBox;
                _currentTimeSpinBox->setTimeObject(timeObject);
                _currentTimeSpinBox->setToolTip("Current time");
                _durationLabel = new qtwidget::TimeLabel;
                _durationLabel->setTimeObject(timeObject);
                _durationLabel->setToolTip("Duration");
                auto toolBar = new QToolBar;
                toolBar->setFloatable(false);
                toolBar->setMovable(false);
                toolBar->addAction(_stopAction);
                toolBar->addAction(_forwardAction);
                toolBar->addWidget(_currentTimeSpinBox);
                toolBar->addWidget(_durationLabel);
                addToolBar(Qt::BottomToolBarArea, toolBar);

                // Update the widget.
                _currentTimeUpdate(time::invalidTime);
                _durationUpdate(time::invalidTime);
                _playbackUpdate(timeline::Playback::Stop);

                // Setup connections.
                connect(
                    _playbackActionGroup,
                    &QActionGroup::triggered,
                    [this](QAction* action)
                    {
                        if (_player)
                        {
                            const timeline::Playback playback = action == _stopAction ?
                                timeline::Playback::Stop :
                                timeline::Playback::Forward;
                            _player->setPlayback(playback);
                            _playbackUpdate(playback);
                        }
                    });

                connect(
                    _currentTimeSpinBox,
                    &qtwidget::TimeSpinBox::valueChanged,
                    [this](const OTIO_NS::RationalTime& value)
                    {
                        if (_player)
                        {
                            _player->setPlayback(timeline::Playback::Stop);
                            _player->seek(value);
                        }
                    });
            }

            MainWindow::~MainWindow()
            {}

            void MainWindow::setPlayer(const QSharedPointer<qt::PlayerObject>& player)
            {
                disconnect(_currentTimeConnection);
                disconnect(_playbackConnection);

                _player = player;
                _viewport->setPlayer(player);
                _timelineWidget->setPlayer(player ? player->player() : nullptr);

                _currentTimeUpdate(player ? player->currentTime() : time::invalidTime);
                _durationUpdate(player ? player->timeRange().duration() : time::invalidTime);
                _playbackUpdate(player ? player->playback() : timeline::Playback::Stop);

                if (player)
                {
                    _currentTimeConnection = connect(
                        player.get(),
                        &qt::PlayerObject::currentTimeChanged,
                        [this](const OTIO_NS::RationalTime& value)
                        {
                            _currentTimeUpdate(value);
                        });
                    _playbackConnection = connect(
                        player.get(),
                        &qt::PlayerObject::playbackChanged,
                        [this](timeline::Playback value)
                        {
                            _playbackUpdate(value);
                        });
                }
            }

            void MainWindow::_currentTimeUpdate(const OTIO_NS::RationalTime& value)
            {
                QSignalBlocker blocker(_currentTimeSpinBox);
                _currentTimeSpinBox->setValue(value);
            }

            void MainWindow::_durationUpdate(const OTIO_NS::RationalTime& value)
            {
                _durationLabel->setValue(value);
            }

            void MainWindow::_playbackUpdate(timeline::Playback playback)
            {
                {
                    QSignalBlocker blocker(_stopAction);
                    _stopAction->setChecked(timeline::Playback::Stop == playback);
                }
                {
                    QSignalBlocker blocker(_forwardAction);
                    _forwardAction->setChecked(timeline::Playback::Forward == playback);
                }
            }
        }
    }
}
