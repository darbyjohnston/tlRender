// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2025 Darby Johnston
// All rights reserved.

#pragma once

#include <tlQt/TimelinePlayer.h>

#include <QStatusBar>

#include <memory>

namespace tl
{
    namespace play_qt
    {
        class App;

        //! Status bar widget.
        class StatusBar : public QStatusBar
        {
            Q_OBJECT

        public:
            StatusBar(App*, QWidget* parent = nullptr);

            virtual ~StatusBar();

        private:
            void _playerUpdate(const QSharedPointer<qt::TimelinePlayer>&);

            DTK_PRIVATE();
        };
    }
}
