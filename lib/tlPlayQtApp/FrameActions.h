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

        //! Frame actions.
        class FrameActions : public QObject
        {
            Q_OBJECT

        public:
            FrameActions(App*, QObject* parent = nullptr);

            virtual ~FrameActions();

            //! Get the actions.
            const QMap<QString, QAction*>& actions() const;

            //! Get the menu.
            QMenu* menu() const;

            //! Set the timeline players.
            void setTimelinePlayers(const QVector<QSharedPointer<qt::TimelinePlayer> >&);

        private:
            void _actionsUpdate();

            TLRENDER_PRIVATE();
        };
    }
}
