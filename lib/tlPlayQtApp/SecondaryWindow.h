// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2025 Darby Johnston
// All rights reserved.

#pragma once

#include <tlQt/TimelinePlayer.h>

#include <QWidget>

namespace tl
{
    namespace qtwidget
    {
        class TimelineViewport;
    }

    namespace play_qt
    {
        class App;

        //! Secondary window.
        class SecondaryWindow : public QWidget
        {
            Q_OBJECT

        public:
            SecondaryWindow(App*, QWidget* parent = nullptr);

            virtual ~SecondaryWindow();

            //! Set the view.
            void setView(
                const dtk::V2I& pos,
                double          zoom,
                bool            frame);

        private:
            DTK_PRIVATE();
        };
    }
}
