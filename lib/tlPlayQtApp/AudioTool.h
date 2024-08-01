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
        //! Audio device widget.
        class AudioDeviceWidget : public QWidget
        {
            Q_OBJECT

        public:
            AudioDeviceWidget(App*, QWidget* parent = nullptr);

            virtual ~AudioDeviceWidget();

        private:
            TLRENDER_PRIVATE();
        };

        //! Audio offset widget.
        class AudioOffsetWidget : public QWidget
        {
            Q_OBJECT

        public:
            AudioOffsetWidget(App*, QWidget* parent = nullptr);

            virtual ~AudioOffsetWidget();

        private:
            TLRENDER_PRIVATE();
        };

        //! Audio tool.
        class AudioTool : public IToolWidget
        {
            Q_OBJECT

        public:
            AudioTool(App*, QWidget* parent = nullptr);

            virtual ~AudioTool();

        private:
            TLRENDER_PRIVATE();
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
