// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021 Darby Johnston
// All rights reserved.

#pragma once

#include <tlrQt/TimelineObject.h>
#include <tlrQt/TimelineWidget.h>

#include <QAction>
#include <QMainWindow>
#include <QPointer>

namespace tlr
{
    //! Main window.
    class MainWindow : public QMainWindow
    {
        Q_OBJECT

    public:
        MainWindow(QWidget* parent = nullptr);

        //! Set the timeline.
        void setTimeline(qt::TimelineObject*);

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
        void _playbackCallback(tlr::timeline::Playback);
        void _loopCallback(tlr::timeline::Loop);
        void _stopCallback();
        void _forwardCallback();
        void _togglePlaybackCallback();
        void _startFrameCallback();
        void _endFrameCallback();
        void _prevFrameCallback();
        void _nextFrameCallback();

    private:
        void _playbackUpdate();
        void _timelineUpdate();

        QPointer<qt::TimelineObject> _timeline;
        QMap<std::string, QPointer<QAction> > _actions;
        QPointer<qt::TimelineWidget> _timelineWidget;
    };
}
