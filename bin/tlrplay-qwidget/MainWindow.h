// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021 Darby Johnston
// All rights reserved.

#pragma once

#include "SettingsObject.h"

#include <tlrQt/TimelinePlayer.h>
#include <tlrQt/TimelineViewport.h>
#include <tlrQt/TimelineWidget.h>

#include <QAction>
#include <QActionGroup>
#include <QMainWindow>
#include <QTabWidget>

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

    protected:
        void closeEvent(QCloseEvent*) override;
        void dragEnterEvent(QDragEnterEvent*) override;
        void dragMoveEvent(QDragMoveEvent*) override;
        void dragLeaveEvent(QDragLeaveEvent*) override;
        void dropEvent(QDropEvent*) override;

    private Q_SLOTS:
        void _openCallback();
        void _openedCallback(tlr::qt::TimelinePlayer*);
        void _closeCallback();
        void _closeAllCallback();
        void _closedCallback(tlr::qt::TimelinePlayer*);
        void _recentFilesCallback(QAction*);
        void _recentFilesCallback();
        void _settingsVisibleCallback(bool);
        void _currentTabCallback(int);
        void _closeTabCallback(int);
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
        void _framePrevCallback();
        void _framePrevX10Callback();
        void _framePrevX100Callback();
        void _frameNextCallback();
        void _frameNextX10Callback();
        void _frameNextX100Callback();
        void _clipPrevCallback();
        void _clipNextCallback();

    private:
        void _setCurrentTimeline(qt::TimelinePlayer*);

        void _recentFilesUpdate();
        void _playbackUpdate();
        void _timelineUpdate();

        QList<qt::TimelinePlayer*> _timelinePlayers;
        qt::TimelinePlayer* _currentTimelinePlayer = nullptr;
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
        QTabWidget* _tabWidget = nullptr;
        qt::TimelineWidget* _timelineWidget = nullptr;
        SettingsObject* _settingsObject = nullptr;
        qt::TimeObject* _timeObject = nullptr;
    };
}
