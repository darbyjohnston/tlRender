// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2025 Darby Johnston
// All rights reserved.

#pragma once

#include <dtk/core/Util.h>

#include <QAction>
#include <QObject>
#include <QMenu>

#include <memory>

namespace tl
{
    namespace play_qt
    {
        class App;

        //! Audio actions.
        class AudioActions : public QObject
        {
            Q_OBJECT

        public:
            AudioActions(App*, QObject* parent = nullptr);

            virtual ~AudioActions();

            //! Get the actions.
            const QMap<QString, QAction*>& actions() const;

            //! Get the menu.
            QMenu* menu() const;

        private:
            void _actionsUpdate();

            DTK_PRIVATE();
        };
    }
}
