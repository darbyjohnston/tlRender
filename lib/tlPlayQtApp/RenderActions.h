// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2023 Darby Johnston
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
    namespace play_qt
    {
        class App;

        //! Render actions.
        class RenderActions : public QObject
        {
            Q_OBJECT

        public:
            RenderActions(App*, QObject* parent = nullptr);

            ~RenderActions() override;

            //! Get the actions.
            const QMap<QString, QAction*>& actions() const;

            //! Get the menu.
            QMenu* menu() const;

            //! Set the image options.
            void setImageOptions(const timeline::ImageOptions&);

            //! Set the display options.
            void setDisplayOptions(const timeline::DisplayOptions&);

        private Q_SLOTS:

        private:
            void _actionsUpdate();

            TLRENDER_PRIVATE();
        };
    }
}
