// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021 Darby Johnston
// All rights reserved.

#include "MainWindow.h"

#include <QDragEnterEvent>
#include <QDragMoveEvent>
#include <QFontDatabase>
#include <QHBoxLayout>
#include <QMenuBar>
#include <QMimeData>
#include <QStyle>
#include <QToolBar>

namespace tlr
{
    MainWindow::MainWindow(QWidget* parent) :
        QMainWindow(parent)
    {
        setAcceptDrops(true);

        _actions["File/Open"] = new QAction;
        _actions["File/Open"]->setText("Open");
        _actions["File/Open"]->setShortcut(QKeySequence::Open);
        _actions["File/Close"] = new QAction;
        _actions["File/Close"]->setText("Close");
        _actions["File/Close"]->setShortcut(QKeySequence::Close);
        _actions["File/Exit"] = new QAction;
        _actions["File/Exit"]->setText("Exit");
        _actions["File/Exit"]->setShortcut(QKeySequence::Quit);

        _actions["Playback/Stop"] = new QAction;
        _actions["Playback/Stop"]->setCheckable(true);
        _actions["Playback/Stop"]->setText("Stop Playback");
        _actions["Playback/Stop"]->setIcon(style()->standardIcon(QStyle::SP_MediaStop));
        _actions["Playback/Stop"]->setToolTip("Stop playback");
        _actions["Playback/Forward"] = new QAction;
        _actions["Playback/Forward"]->setCheckable(true);
        _actions["Playback/Forward"]->setText("Forward Playback");
        _actions["Playback/Forward"]->setIcon(style()->standardIcon(QStyle::SP_MediaPlay));
        _actions["Playback/Forward"]->setToolTip("Forward playback");
        _actions["Playback/Toggle"] = new QAction;
        _actions["Playback/Toggle"]->setText("Toggle Playback");
        _actions["Playback/Toggle"]->setShortcut(QKeySequence(Qt::Key_Space));
        _actions["Playback/Toggle"]->setToolTip("Toggle playback");

        _actions["Playback/StartFrame"] = new QAction;
        _actions["Playback/StartFrame"]->setText("Start Frame");
        _actions["Playback/StartFrame"]->setShortcut(QKeySequence(Qt::Key_Home));
        _actions["Playback/EndFrame"] = new QAction;
        _actions["Playback/EndFrame"]->setText("End Frame");
        _actions["Playback/EndFrame"]->setShortcut(QKeySequence(Qt::Key_End));
        _actions["Playback/PrevFrame"] = new QAction;
        _actions["Playback/PrevFrame"]->setText("Previous Frame");
        _actions["Playback/PrevFrame"]->setShortcut(QKeySequence(Qt::Key_Left));
        _actions["Playback/NextFrame"] = new QAction;
        _actions["Playback/NextFrame"]->setText("Next Frame");
        _actions["Playback/NextFrame"]->setShortcut(QKeySequence(Qt::Key_Right));

        auto fileMenu = new QMenu;
        fileMenu->setTitle("&File");
        fileMenu->addAction(_actions["File/Open"]);
        fileMenu->addAction(_actions["File/Close"]);
        fileMenu->addSeparator();
        fileMenu->addAction(_actions["File/Exit"]);

        auto playbackMenu = new QMenu;
        playbackMenu->setTitle("&Playback");
        playbackMenu->addAction(_actions["Playback/Toggle"]);
        playbackMenu->addSeparator();
        playbackMenu->addAction(_actions["Playback/StartFrame"]);
        playbackMenu->addAction(_actions["Playback/EndFrame"]);
        playbackMenu->addAction(_actions["Playback/PrevFrame"]);
        playbackMenu->addAction(_actions["Playback/NextFrame"]);

        auto menuBar = new QMenuBar;
        menuBar->addMenu(fileMenu);
        menuBar->addMenu(playbackMenu);
        setMenuBar(menuBar);

        const QFont fixedFont = QFontDatabase::systemFont(QFontDatabase::FixedFont);
        _currentTimeLabel = new QLabel;
        _currentTimeLabel->setMargin(10);
        _currentTimeLabel->setFont(fixedFont);
        _currentTimeLabel->setText("00:00:00:00");
        _currentTimeLabel->setToolTip("Current time");

        _timeSlider = new QSlider(Qt::Orientation::Horizontal);
        _timeSlider->setToolTip("Time slider");

        _durationLabel = new QLabel;
        _durationLabel->setMargin(10);
        _durationLabel->setFont(fixedFont);
        _durationLabel->setText("00:00:00:00");
        _durationLabel->setToolTip("Duration");

        auto playbackToolBar = new QToolBar;
        playbackToolBar->setMovable(false);
        playbackToolBar->setFloatable(false);
        playbackToolBar->addAction(_actions["Playback/Stop"]);
        playbackToolBar->addAction(_actions["Playback/Forward"]);
        playbackToolBar->addWidget(_currentTimeLabel);
        playbackToolBar->addWidget(_timeSlider);
        playbackToolBar->addWidget(_durationLabel);
        addToolBar(Qt::ToolBarArea::BottomToolBarArea, playbackToolBar);

        _playbackUpdate();
        _timelineUpdate();

        connect(
            _actions["File/Open"],
            SIGNAL(triggered()),
            SIGNAL(fileOpen()));
        connect(
            _actions["File/Close"],
            SIGNAL(triggered()),
            SIGNAL(fileClose()));
        connect(
            _actions["File/Exit"],
            SIGNAL(triggered()),
            SIGNAL(exit()));

        connect(
            _actions["Playback/Stop"],
            SIGNAL(triggered()),
            SLOT(_stopCallback()));
        connect(
            _actions["Playback/Forward"],
            SIGNAL(triggered()),
            SLOT(_forwardCallback()));
        connect(
            _actions["Playback/Toggle"],
            SIGNAL(triggered()),
            SLOT(_togglePlaybackCallback()));
        connect(
            _actions["Playback/StartFrame"],
            SIGNAL(triggered()),
            SLOT(_startFrameCallback()));
        connect(
            _actions["Playback/EndFrame"],
            SIGNAL(triggered()),
            SLOT(_endFrameCallback()));
        connect(
            _actions["Playback/PrevFrame"],
            SIGNAL(triggered()),
            SLOT(_prevFrameCallback()));
        connect(
            _actions["Playback/NextFrame"],
            SIGNAL(triggered()),
            SLOT(_nextFrameCallback()));
        connect(
            _timeSlider,
            SIGNAL(valueChanged(int)),
            SLOT(_timeSliderCallback(int)));
    }

    void MainWindow::setTimeline(const std::shared_ptr<timeline::Timeline>& timeline)
    {
        if (timeline == _timeline)
            return;
        _timeline = timeline;
        _timelineUpdate();
    }

    void MainWindow::dragEnterEvent(QDragEnterEvent* event)
    {
        const QMimeData* mimeData = event->mimeData();
        if (mimeData->hasUrls())
        {
            const auto urlList = mimeData->urls();
            if (1 == urlList.size())
            {
                event->acceptProposedAction();
            }
        }
    }

    void MainWindow::dragMoveEvent(QDragMoveEvent* event)
    {
        const QMimeData* mimeData = event->mimeData();
        if (mimeData->hasUrls())
        {
            const auto urlList = mimeData->urls();
            if (1 == urlList.size())
            {
                event->acceptProposedAction();
            }
        }
    }

    void MainWindow::dragLeaveEvent(QDragLeaveEvent* event)
    {
        event->accept();
    }

    void MainWindow::dropEvent(QDropEvent* event)
    {
        const QMimeData* mimeData = event->mimeData();
        if (mimeData->hasUrls())
        {
            const auto urlList = mimeData->urls();
            if (1 == urlList.size())
            {
                fileOpen(urlList[0].toLocalFile().toLatin1().data());
            }
        }
    }

    void MainWindow::_stopCallback()
    {
        if (_timeline)
        {
            _timeline->setPlayback(timeline::Playback::Stop);
            _playbackUpdate();
        }
    }

    void MainWindow::_forwardCallback()
    {
        if (_timeline)
        {
            _timeline->setPlayback(timeline::Playback::Forward);
            _playbackUpdate();
        }
    }

    void MainWindow::_togglePlaybackCallback()
    {
        if (_timeline)
        {
            _timeline->setPlayback(
                timeline::Playback::Forward == _timeline->observePlayback()->get() ?
                timeline::Playback::Stop :
                timeline::Playback::Forward);
        }
    }

    void MainWindow::_startFrameCallback()
    {
        if (_timeline)
        {
            _timeline->setPlayback(timeline::Playback::Stop);
            const auto duration = _timeline->getDuration();
            _timeline->seek(otime::RationalTime(0, duration.rate()));
        }
    }

    void MainWindow::_endFrameCallback()
    {
        if (_timeline)
        {
            _timeline->setPlayback(timeline::Playback::Stop);
            const auto duration = _timeline->getDuration();
            _timeline->seek(otime::RationalTime(duration.value() - 1, duration.rate()));
        }
    }

    void MainWindow::_prevFrameCallback()
    {
        if (_timeline)
        {
            _timeline->setPlayback(timeline::Playback::Stop);
            const auto duration = _timeline->getDuration();
            const auto currentTime = _timeline->observeCurrentTime()->get();
            _timeline->seek(otime::RationalTime(currentTime.value() - 1, duration.rate()));
        }
    }

    void MainWindow::_nextFrameCallback()
    {
        if (_timeline)
        {
            _timeline->setPlayback(timeline::Playback::Stop);
            const auto duration = _timeline->getDuration();
            const auto currentTime = _timeline->observeCurrentTime()->get();
            _timeline->seek(otime::RationalTime(currentTime.value() + 1, duration.rate()));
        }
    }

    void MainWindow::_timeSliderCallback(int value)
    {
        if (_timeline)
        {
            _timeline->setPlayback(timeline::Playback::Stop);
            _timeline->seek(otime::RationalTime(value, _timeline->getDuration().rate()));
        }
    }

    void MainWindow::_playbackUpdate()
    {
        timeline::Playback playback = timeline::Playback::Stop;
        if (_timeline)
        {
            playback = _timeline->observePlayback()->get();
        }
        _actions["Playback/Stop"]->setChecked(timeline::Playback::Stop == playback);
        _actions["Playback/Forward"]->setChecked(timeline::Playback::Forward == playback);
    }

    void MainWindow::_timelineUpdate()
    {
        if (_timeline)
        {
            _actions["File/Close"]->setEnabled(true);

            _actions["Playback/Stop"]->setEnabled(true);
            _actions["Playback/Forward"]->setEnabled(true);
            _actions["Playback/StartFrame"]->setEnabled(true);
            _actions["Playback/EndFrame"]->setEnabled(true);
            _actions["Playback/PrevFrame"]->setEnabled(true);
            _actions["Playback/NextFrame"]->setEnabled(true);

            const otime::RationalTime& duration = _timeline->getDuration();
            _timeSlider->setRange(0, duration.value() > 0 ? duration.value() - 1 : 0);
            _timeSlider->setEnabled(true);

            otime::ErrorStatus errorStatus;
            std::string label = duration.to_timecode(&errorStatus);
            if (errorStatus != otime::ErrorStatus::OK)
            {
                throw std::runtime_error(errorStatus.details);
            }
            _durationLabel->setText(label.c_str());

            _currentTimeObserver = Observer::Value<otime::RationalTime>::create(
                _timeline->observeCurrentTime(),
                [this](const otime::RationalTime& value)
                {
                    otime::ErrorStatus errorStatus;
                    std::string label = value.to_timecode(&errorStatus);
                    if (errorStatus != otime::ErrorStatus::OK)
                    {
                        throw std::runtime_error(errorStatus.details);
                    }
                    _currentTimeLabel->setText(label.c_str());

                    const QSignalBlocker blocker(_timeSlider);
                    _timeSlider->setValue(value.value());
                });

            _playbackObserver = Observer::Value<timeline::Playback>::create(
                _timeline->observePlayback(),
                [this](timeline::Playback value)
                {
                    _playbackUpdate();
                });

            _loopObserver = Observer::Value<timeline::Loop>::create(
                _timeline->observeLoop(),
                [this](timeline::Loop value)
                {
                });
        }
        else
        {
            _actions["File/Close"]->setEnabled(false);

            _actions["Playback/Stop"]->setChecked(true);
            _actions["Playback/Stop"]->setEnabled(false);
            _actions["Playback/Forward"]->setEnabled(false);
            _actions["Playback/StartFrame"]->setEnabled(false);
            _actions["Playback/EndFrame"]->setEnabled(false);
            _actions["Playback/PrevFrame"]->setEnabled(false);
            _actions["Playback/NextFrame"]->setEnabled(false);

            _currentTimeLabel->setText("00:00:00:00");

            _timeSlider->setRange(0, 0);
            _timeSlider->setValue(0);
            _timeSlider->setEnabled(false);

            _durationLabel->setText("00:00:00:00");

            _currentTimeObserver.reset();
            _playbackObserver.reset();
            _loopObserver.reset();
        }
    }
}
