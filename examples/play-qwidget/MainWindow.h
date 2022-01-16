// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2022 Darby Johnston
// All rights reserved.

#pragma once

#include "AudioTool.h"
#include "ImageTool.h"
#include "LayersTool.h"
#include "SecondaryWindow.h"
#include "SettingsObject.h"

#include <tlrQWidget/TimelineWidget.h>

#include <tlrQt/TimelinePlayer.h>

#include <tlrCore/OCIO.h>

#include <QAction>
#include <QActionGroup>
#include <QMainWindow>

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
            const std::shared_ptr<core::Context>&,
            QWidget* parent = nullptr);

        ~MainWindow() override;

        void setTimelinePlayer(qt::TimelinePlayer*);
        void setColorConfig(const imaging::ColorConfig&);

    protected:
        void closeEvent(QCloseEvent*) override;
        void dragEnterEvent(QDragEnterEvent*) override;
        void dragMoveEvent(QDragMoveEvent*) override;
        void dragLeaveEvent(QDragLeaveEvent*) override;
        void dropEvent(QDropEvent*) override;

    private Q_SLOTS:
        void _openCallback();
        void _openWithAudioCallback();
        void _closeCallback();
        void _closeAllCallback();
        void _recentFilesCallback(QAction*);
        void _recentFilesCallback();
        void _nextCallback();
        void _prevCallback();
        void _layersCallback(QAction*);
        void _layersCallback(int);
        void _resize1280x720Callback();
        void _resize1920x1080Callback();
        void _fullScreenCallback();
        void _secondaryWindowCallback(bool);
        void _secondaryWindowDestroyedCallback();
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
        void _imageOptionsCallback(const tlr::render::ImageOptions&);
        void _audioOffsetCallback(double);
        void _settingsCallback();
        void _saveSettingsCallback();

    private:
        void _recentFilesUpdate();
        void _layersUpdate();
        void _playbackUpdate();
        void _timelineUpdate();
        void _settingsUpdate();

        std::weak_ptr<core::Context> _context;
        qt::TimelinePlayer* _timelinePlayer = nullptr;
        qwidget::TimelineWidget* _timelineWidget = nullptr;
        AudioTool* _audioTool = nullptr;
        ImageTool* _imageTool = nullptr;
        LayersTool* _layersTool = nullptr;
        QMap<QString, QAction*> _actions;
        QActionGroup* _recentFilesActionGroup = nullptr;
        QMap<QAction*, QString> _actionToRecentFile;
        QMenu* _recentFilesMenu = nullptr;
        QActionGroup* _layersActionGroup = nullptr;
        QMap<QAction*, uint16_t> _actionToLayer;
        QMenu* _layersMenu = nullptr;
        QActionGroup* _playbackActionGroup = nullptr;
        QMap<QAction*, timeline::Playback> _actionToPlayback;
        QMap<timeline::Playback, QAction*> _playbackToActions;
        QActionGroup* _loopActionGroup = nullptr;
        QMap<QAction*, timeline::Loop> _actionToLoop;
        QMap<timeline::Loop, QAction*> _loopToActions;
        SecondaryWindow* _secondaryWindow = nullptr;
        imaging::ColorConfig _colorConfig;
        SettingsObject* _settingsObject = nullptr;
        qt::TimeObject* _timeObject = nullptr;
    };
}
