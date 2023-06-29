// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2023 Darby Johnston
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

    namespace qtplay
    {
        class App;

        //! Secondary window.
        class SecondaryWindow : public QWidget
        {
            Q_OBJECT

        public:
            SecondaryWindow(
                App*,
                QWidget* parent = nullptr);

            ~SecondaryWindow() override;

            //! Get the viewport.
            qtwidget::TimelineViewport* viewport() const;

        protected:
            void keyPressEvent(QKeyEvent*) override;

        private:
            TLRENDER_PRIVATE();
        };
    }
}
