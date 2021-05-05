// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021 Darby Johnston
// All rights reserved.

#pragma once

#include <tlrQt/TimelineObject.h>
#include <tlrQt/TimelineViewport.h>
#include <tlrQt/TimelineWidget.h>

#include <QAction>
#include <QActionGroup>
#include <QMainWindow>
#include <QPointer>

namespace tlr
{
    //! Main window.
    class MainWindow : public QMainWindow
    {
        Q_OBJECT

    public:
        MainWindow(qt::TimeObject*, QWidget* parent = nullptr);

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
        void _timeUnitsCallback(QAction*);
        void _timeUnitsCallback(qt::TimeObject::Units);

    private:
        void _playbackUpdate();
        void _timelineUpdate();

        QPointer<qt::TimelineObject> _timeline;
        QMap<std::string, QPointer<QAction> > _actions;
        QPointer<QActionGroup> _timeUnitsActionGroup;
        QMap<QAction*, qt::TimeObject::Units> _actionToUnits;
        QMap<qt::TimeObject::Units, QAction*> _unitsToActions;
        QPointer<qt::TimelineViewport> _viewport;
        QPointer<qt::TimelineWidget> _timelineWidget;
        QPointer<qt::TimeObject> _timeObject;
    };
}
