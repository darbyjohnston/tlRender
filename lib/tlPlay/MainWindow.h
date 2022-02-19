// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2022 Darby Johnston
// All rights reserved.

#pragma once

#include <tlQt/TimelinePlayer.h>

#include <tlCore/IRender.h>

#include <QMainWindow>

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

            //! Set the timeline players.
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
            void _compareCallback(QAction*);
            void _secondaryWindowCallback(bool);
            void _secondaryWindowDestroyedCallback();
            void _yuvRangeCallback(QAction*);
            void _channelsCallback(QAction*);
            void _alphaBlendCallback(QAction*);
            void _playbackCallback(QAction*);
            void _playbackCallback(tl::timeline::Playback);
            void _loopCallback(QAction*);
            void _loopCallback(tl::timeline::Loop);

        private:
            void _recentFilesUpdate();
            void _widgetUpdate();

            TLRENDER_PRIVATE();
        };
    }
}
