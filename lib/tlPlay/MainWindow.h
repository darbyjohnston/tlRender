// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2022 Darby Johnston
// All rights reserved.

#pragma once

#include <tlPlay/AudioTool.h>
#include <tlPlay/ColorTool.h>
#include <tlPlay/CompareTool.h>
#include <tlPlay/FilesModel.h>
#include <tlPlay/FilesTool.h>
#include <tlPlay/ImageTool.h>
#include <tlPlay/InfoTool.h>
#include <tlPlay/MessagesTool.h>
#include <tlPlay/SecondaryWindow.h>
#include <tlPlay/SettingsObject.h>
#include <tlPlay/SettingsTool.h>
#include <tlPlay/SystemLogTool.h>

#include <tlQWidget/TimelineWidget.h>

#include <tlQt/TimelinePlayer.h>

#include <tlCore/OCIO.h>

#include <QAction>
#include <QActionGroup>
#include <QLabel>
#include <QMainWindow>
#include <QStatusBar>

namespace tl
{
    namespace play
    {
        class App;

        //! Main window.
        class MainWindow : public QMainWindow
        {
            Q_OBJECT

        public:
            MainWindow(App*, QWidget* parent = nullptr);

            ~MainWindow() override;

            void setImageOptions(const std::vector<render::ImageOptions>&);

            void setTimelinePlayers(const std::vector<qt::TimelinePlayer*>&);

        protected:
            void closeEvent(QCloseEvent*) override;
            void dragEnterEvent(QDragEnterEvent*) override;
            void dragMoveEvent(QDragMoveEvent*) override;
            void dragLeaveEvent(QDragLeaveEvent*) override;
            void dropEvent(QDropEvent*) override;

        private Q_SLOTS:
            void _recentFilesCallback(QAction*);
            void _recentFilesCallback();
            void _secondaryWindowCallback(bool);
            void _secondaryWindowDestroyedCallback();
            void _channelsCallback(QAction*);
            void _playbackCallback(QAction*);
            void _playbackCallback(tl::timeline::Playback);
            void _loopCallback(QAction*);
            void _loopCallback(tl::timeline::Loop);
            void _imageOptionsCallback(const tl::render::ImageOptions&);
            void _imageOptionsCallback(const std::vector<tl::render::ImageOptions>&);
            void _compareOptionsCallback(const tl::render::CompareOptions&);
            void _compareOptionsCallback2(const tl::render::CompareOptions&);

        private:
            void _recentFilesUpdate();
            void _widgetUpdate();

            App* _app = nullptr;

            std::vector<qt::TimelinePlayer*> _timelinePlayers;
            bool _floatOnTop = false;
            bool _secondaryFloatOnTop = false;
            imaging::ColorConfig _colorConfig;
            std::vector<render::ImageOptions> _imageOptions;
            render::CompareOptions _compareOptions;

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

            qwidget::TimelineWidget* _timelineWidget = nullptr;
            FilesTool* _filesTool = nullptr;
            CompareTool* _compareTool = nullptr;
            ColorTool* _colorTool = nullptr;
            ImageTool* _imageTool = nullptr;
            InfoTool* _infoTool = nullptr;
            AudioTool* _audioTool = nullptr;
            SettingsTool* _settingsTool = nullptr;
            MessagesTool* _messagesTool = nullptr;
            SystemLogTool* _systemLogTool = nullptr;
            QLabel* _infoLabel = nullptr;
            QStatusBar* _statusBar = nullptr;
            SecondaryWindow* _secondaryWindow = nullptr;

            std::shared_ptr<observer::ListObserver<std::shared_ptr<FilesModelItem> > > _filesObserver;
            std::shared_ptr<observer::ListObserver<render::ImageOptions> > _imageOptionsObserver;
            std::shared_ptr<observer::ValueObserver<render::CompareOptions> > _compareOptionsObserver;
            std::shared_ptr<observer::ValueObserver<imaging::ColorConfig> > _colorConfigObserver;
            std::shared_ptr<observer::ValueObserver<core::LogItem> > _logObserver;
        };
    }
}
