// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2022 Darby Johnston
// All rights reserved.

#pragma once

#include <tlrQWidget/TimelineViewport.h>

#include <QWidget>

namespace tlr
{
    //! Secondary window.
    class SecondaryWindow : public QWidget
    {
        Q_OBJECT

    public:
        SecondaryWindow(
            const std::shared_ptr<core::Context>&,
            QWidget* parent = nullptr);

        //! Set the color configuration.
        void setColorConfig(const imaging::ColorConfig&);

        //! Set the timeline player.
        void setTimelinePlayer(qt::TimelinePlayer*);

    protected:
        void keyPressEvent(QKeyEvent*) override;

    private:
        qwidget::TimelineViewport* _viewport = nullptr;
    };
}
