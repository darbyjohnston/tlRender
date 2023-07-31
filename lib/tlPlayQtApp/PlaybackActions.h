// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2023 Darby Johnston
// All rights reserved.

#pragma once

#include <tlQt/TimelinePlayer.h>

#include <QAction>
#include <QObject>
#include <QMenu>

#include <memory>

namespace tl
{
    namespace play_qt
    {
        class App;

        //! Playback actions.
        class PlaybackActions : public QObject
        {
            Q_OBJECT

        public:
            PlaybackActions(App*, QObject* parent = nullptr);

            virtual ~PlaybackActions();

            //! Get the actions.
            const QMap<QString, QAction*>& actions() const;

            //! Get the menu.
            QMenu* menu() const;

            //! Get the speed menu.
            QMenu* speedMenu() const;

            //! Set the timeline players.
            void setTimelinePlayers(const QVector<QPointer<qt::TimelinePlayer> >&);

        private Q_SLOTS:
            void _playbackCallback(tl::timeline::Playback);
            void _loopCallback(tl::timeline::Loop);

        private:
            void _actionsUpdate();

            TLRENDER_PRIVATE();
        };
    }
}
