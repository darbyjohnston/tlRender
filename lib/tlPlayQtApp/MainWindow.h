// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2023 Darby Johnston
// All rights reserved.

#pragma once

#include <tlQt/TimelinePlayer.h>

#include <QMainWindow>

namespace tl
{
    namespace play_qt
    {
        class App;

        //! Main window.
        class MainWindow : public QMainWindow
        {
            Q_OBJECT

        public:
            MainWindow(App*, QWidget* parent = nullptr);

            virtual ~MainWindow();

            //! Set the timeline players.
            void setTimelinePlayers(const QVector<QSharedPointer<qt::TimelinePlayer> >&);

        protected:
            void closeEvent(QCloseEvent*) override;
            void dragEnterEvent(QDragEnterEvent*) override;
            void dragMoveEvent(QDragMoveEvent*) override;
            void dragLeaveEvent(QDragLeaveEvent*) override;
            void dropEvent(QDropEvent*) override;

        private Q_SLOTS:
            void _secondaryWindowCallback(bool);
            void _secondaryWindowDestroyedCallback();
            void _speedCallback(double);
            void _playbackCallback(tl::timeline::Playback);
            void _currentTimeCallback(const otime::RationalTime&);
            void _volumeCallback(int);

        private:
            void _timelinePlayersUpdate();
            void _widgetUpdate();

            TLRENDER_PRIVATE();
        };
    }
}
