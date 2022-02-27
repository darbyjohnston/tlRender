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

        //! Window actions.
        class WindowActions : public QObject
        {
            Q_OBJECT

        public:
            WindowActions(App*, QObject* parent = nullptr);

            ~WindowActions() override;

            //! Get the actions.
            const QMap<QString, QAction*>& actions() const;

            //! Get the menu.
            QMenu* menu() const;

            //! Set the timeline players.
            void setTimelinePlayers(const std::vector<qt::TimelinePlayer*>&);

        Q_SIGNALS:
            //! This signal is emitted to resize the window.
            void resize(const tl::imaging::Size&);

        private:
            void _actionsUpdate();

            TLRENDER_PRIVATE();
        };
    }
}
