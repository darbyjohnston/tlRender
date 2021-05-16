// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021 Darby Johnston
// All rights reserved.

#pragma once

#include "SettingsObject.h"

#include <tlrQt/FilmstripWidget.h>
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
        MainWindow(
            SettingsObject*,
            qt::TimeObject*,
            QWidget* parent = nullptr);

        //! Set the timeline object.
        void setTimeline(qt::TimelineObject*);

    Q_SIGNALS:
        void fileOpen();
        void fileOpen(const QString&);
        void fileClose();
        void exit();

    protected:
        void closeEvent(QCloseEvent*) override;
        void dragEnterEvent(QDragEnterEvent*) override;
        void dragMoveEvent(QDragMoveEvent*) override;
        void dragLeaveEvent(QDragLeaveEvent*) override;
        void dropEvent(QDropEvent*) override;

    private Q_SLOTS:
        void _recentFilesCallback(QAction*);
        void _recentFilesCallback();
        void _settingsVisibleCallback(bool);
        void _playbackCallback(QAction*);
        void _playbackCallback(tlr::timeline::Playback);
        void _loopCallback(QAction*);
        void _loopCallback(tlr::timeline::Loop);
        void _stopCallback();
        void _forwardCallback();
        void _reverseCallback();
        void _togglePlaybackCallback();
        void _startCallback();
        void _endCallback();
        void _prevCallback();
        void _nextCallback();

    private:
        void _recentFilesUpdate();
        void _playbackUpdate();
        void _timelineUpdate();

        qt::TimelineObject* _timeline = nullptr;
        QMap<QString, QAction*> _actions;
        QActionGroup* _recentFilesActionGroup = nullptr;
        QMap<QAction*, QString> _actionToRecentFile;
        QMenu* _recentFilesMenu = nullptr;
        QActionGroup* _playbackActionGroup = nullptr;
        QMap<QAction*, timeline::Playback> _actionToPlayback;
        QMap<timeline::Playback, QAction*> _playbackToActions;
        QActionGroup* _loopActionGroup = nullptr;
        QMap<QAction*, timeline::Loop> _actionToLoop;
        QMap<timeline::Loop, QAction*> _loopToActions;
        qt::TimelineViewport* _viewport = nullptr;
        qt::TimelineWidget* _timelineWidget = nullptr;
        qt::FilmstripWidget* _filmstripWidget = nullptr;
        QPointer<SettingsObject> _settingsObject;
        QPointer<qt::TimeObject> _timeObject;
    };
}
