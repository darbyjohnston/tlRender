// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2022 Darby Johnston
// All rights reserved.

#pragma once

#include <tlQt/TimelinePlayer.h>

#include <tlCore/IRender.h>

#include <QWidget>

namespace tl
{
    namespace core
    {
        class Context;
    }

    namespace qwidget
    {
        class TimelineViewport;
    }

    namespace play
    {
        //! Secondary window.
        class SecondaryWindow : public QWidget
        {
            Q_OBJECT

        public:
            SecondaryWindow(
                const std::shared_ptr<core::Context>&,
                QWidget* parent = nullptr);

            ~SecondaryWindow() override;

            //! Get the viewport.
            qwidget::TimelineViewport* viewport() const;

        protected:
            void keyPressEvent(QKeyEvent*) override;

        private:
            TLRENDER_PRIVATE();
        };
    }
}
