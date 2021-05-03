// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021 Darby Johnston
// All rights reserved.

#pragma once

#include <tlrRender/Timeline.h>

#include <QAction>
#include <QMainWindow>
#include <QLabel>
#include <QMap>
#include <QPointer>
#include <QSlider>

namespace tlr
{
    //! Main window.
    class MainWindow : public QMainWindow
    {
        Q_OBJECT

    public:
        MainWindow(QWidget* parent = nullptr);

        //! Set the timeline.
        void setTimeline(const std::shared_ptr<timeline::Timeline>&);

    Q_SIGNALS:
        void fileOpen();
        void fileOpen(const QString&);
        void fileClose();
        void exit();

    protected:
        void dragEnterEvent(QDragEnterEvent*) override;
        void dragMoveEvent(QDragMoveEvent*) override;
        void dragLeaveEvent(QDragLeaveEvent*) override;
        void dropEvent(QDropEvent*) override;

    private Q_SLOTS:
        void _stopCallback();
        void _forwardCallback();
        void _togglePlaybackCallback();
        void _startFrameCallback();
        void _endFrameCallback();
        void _prevFrameCallback();
        void _nextFrameCallback();
        void _timeSliderCallback(int);

    private:
        void _playbackUpdate();
        void _timelineUpdate();

        std::shared_ptr<timeline::Timeline> _timeline;

        QMap<std::string, QPointer<QAction> > _actions;
        QPointer<QLabel> _currentTimeLabel;
        QPointer<QSlider> _timeSlider;
        QPointer<QLabel> _durationLabel;

        std::shared_ptr<Observer::Value<otime::RationalTime> > _currentTimeObserver;
        std::shared_ptr<Observer::Value<timeline::Playback> > _playbackObserver;
        std::shared_ptr<Observer::Value<timeline::Loop> > _loopObserver;
    };
}
