// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2024 Darby Johnston
// All rights reserved.

#pragma once

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

            virtual ~RenderActions();

            //! Get the actions.
            const QMap<QString, QAction*>& actions() const;

            //! Get the menu.
            QMenu* menu() const;

        private Q_SLOTS:

        private:
            void _actionsUpdate();

            TLRENDER_PRIVATE();
        };
    }
}
