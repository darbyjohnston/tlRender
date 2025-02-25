// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2024 Darby Johnston
// All rights reserved.

#pragma once

#include <tlPlayQtApp/IToolWidget.h>

#include <tlQt/TimelinePlayer.h>

#include <QDockWidget>

namespace tl
{
    namespace play_qt
    {
        //! Audio tool.
        class AudioTool : public IToolWidget
        {
            Q_OBJECT

        public:
            AudioTool(App*, QWidget* parent = nullptr);

            virtual ~AudioTool();

        private:
            DTK_PRIVATE();
        };

        //! Audio tool dock widget.
        class AudioDockWidget : public QDockWidget
        {
            Q_OBJECT

        public:
            AudioDockWidget(
                AudioTool*,
                QWidget* parent = nullptr);
        };
    }
}
