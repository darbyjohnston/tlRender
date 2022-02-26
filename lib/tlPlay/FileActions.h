// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2022 Darby Johnston
// All rights reserved.

#pragma once

#include <tlQt/TimelinePlayer.h>

#include <QAction>
#include <QObject>
#include <QMenu>

#include <memory>

namespace tl
{
    namespace play
    {
        class App;

        //! File actions.
        class FileActions : public QObject
        {
            Q_OBJECT

        public:
            FileActions(App*, QObject* parent = nullptr);

            ~FileActions() override;

            //! Get the actions.
            const QMap<QString, QAction*>& actions() const;

            //! Get the menu.
            QMenu* menu() const;

            //! Set the timeline players.
            void setTimelinePlayers(const std::vector<qt::TimelinePlayer*>&);

        private Q_SLOTS:
            void _recentFilesCallback();
        
        private:
            void _recentFilesUpdate();
            void _actionsUpdate();

            TLRENDER_PRIVATE();
        };
    }
}
