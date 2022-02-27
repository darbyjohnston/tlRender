// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2022 Darby Johnston
// All rights reserved.

#pragma once

#include <tlQt/TimelinePlayer.h>

#include <tlTimeline/IRender.h>

#include <QAction>
#include <QObject>
#include <QMenu>

#include <memory>

namespace tl
{
    namespace app
    {
        namespace play
        {
            class App;

            //! Compare actions.
            class CompareActions : public QObject
            {
                Q_OBJECT

            public:
                CompareActions(App*, QObject* parent = nullptr);

                ~CompareActions() override;

                //! Get the actions.
                const QMap<QString, QAction*>& actions() const;

                //! Get the menu.
                QMenu* menu() const;

                //! Set the comparison options.
                void setCompareOptions(const timeline::CompareOptions&);

                //! Set the timeline players.
                void setTimelinePlayers(const std::vector<qt::TimelinePlayer*>&);

            private:
                void _actionsUpdate();

                TLRENDER_PRIVATE();
            };
        }
    }
}
