// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2022 Darby Johnston
// All rights reserved.

#pragma once

#include "AudioTool.h"
#include "CompareTool.h"
#include "FilesModel.h"
#include "FilesTool.h"
#include "ImageTool.h"
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
    class App;

    //! Main window.
    class MainWindow : public QMainWindow
    {
        Q_OBJECT

    public:
        MainWindow(App*, QWidget* parent = nullptr);

        ~MainWindow() override;

        void setColorConfig(const imaging::ColorConfig&);

        void setImageOptions(const std::vector<render::ImageOptions>&);

        void setTimelinePlayers(const std::vector<qt::TimelinePlayer*>&);

    protected:
        void closeEvent(QCloseEvent*) override;
        void dragEnterEvent(QDragEnterEvent*) override;
        void dragMoveEvent(QDragMoveEvent*) override;
        void dragLeaveEvent(QDragLeaveEvent*) override;
        void dropEvent(QDropEvent*) override;

    private Q_SLOTS:
        void _closeCallback();
        void _closeAllCallback();
        void _recentFilesCallback(QAction*);
        void _recentFilesCallback();
        void _nextCallback();
        void _prevCallback();
        void _resize1280x720Callback();
        void _resize1920x1080Callback();
        void _fullScreenCallback();
        void _secondaryWindowCallback(bool);
        void _secondaryWindowDestroyedCallback();
        void _channelsCallback(QAction*);
        void _playbackCallback(QAction*);
        void _playbackCallback(tlr::timeline::Playback);
        void _loopCallback(QAction*);
        void _loopCallback(tlr::timeline::Loop);
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
        void _imageOptionsCallback(const std::vector<tlr::render::ImageOptions>&);
        void _compareOptionsCallback(const tlr::render::CompareOptions&);
        void _compareOptionsCallback2(const tlr::render::CompareOptions&);
        void _audioOffsetCallback(double);

    private:
        void _recentFilesUpdate();
        void _filesCountUpdate();
        void _playbackUpdate();
        void _widgetUpdate();

        App* _app = nullptr;
        std::shared_ptr<observer::ListObserver<std::shared_ptr<FilesModelItem> > > _filesObserver;
        std::shared_ptr<observer::ListObserver<render::ImageOptions> > _imageOptionsObserver;
        std::shared_ptr<observer::ValueObserver<render::CompareOptions> > _compareOptionsObserver;
        std::vector<qt::TimelinePlayer*> _timelinePlayers;
        qwidget::TimelineWidget* _timelineWidget = nullptr;
        FilesTool* _filesTool = nullptr;
        CompareTool* _compareTool = nullptr;
        ImageTool* _imageTool = nullptr;
        AudioTool* _audioTool = nullptr;
        QMap<QString, QAction*> _actions;
        QActionGroup* _recentFilesActionGroup = nullptr;
        QMap<QAction*, QString> _actionToRecentFile;
        QMenu* _recentFilesMenu = nullptr;
        QActionGroup* _channelsActionGroup = nullptr;
        QMap<QAction*, render::Channels> _actionToChannels;
        QMap<render::Channels, QAction*> _channelsToActions;
        QActionGroup* _playbackActionGroup = nullptr;
        QMap<QAction*, timeline::Playback> _actionToPlayback;
        QMap<timeline::Playback, QAction*> _playbackToActions;
        QActionGroup* _loopActionGroup = nullptr;
        QMap<QAction*, timeline::Loop> _actionToLoop;
        QMap<timeline::Loop, QAction*> _loopToActions;
        SecondaryWindow* _secondaryWindow = nullptr;
        imaging::ColorConfig _colorConfig;
        std::vector<render::ImageOptions> _imageOptions;
        render::CompareOptions _compareOptions;
    };
}
