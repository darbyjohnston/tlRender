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

            //! Image actions.
            class ImageActions : public QObject
            {
                Q_OBJECT

            public:
                ImageActions(App*, QObject* parent = nullptr);

                ~ImageActions() override;

                //! Get the actions.
                const QMap<QString, QAction*>& actions() const;

                //! Get the menu.
                QMenu* menu() const;

                //! Set the image options.
                void setImageOptions(const timeline::ImageOptions&);

                //! Set the timeline players.
                void setTimelinePlayers(const std::vector<qt::TimelinePlayer*>&);

            private Q_SLOTS:

            private:
                void _actionsUpdate();

                TLRENDER_PRIVATE();
            };
        }
    }
}
