// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2023 Darby Johnston
// All rights reserved.

#pragma once

#include <tlPlayQtApp/IToolWidget.h>

#include <tlQt/TimelinePlayer.h>

#include <QDockWidget>

namespace tl
{
    namespace play_qt
    {
        //! Audio offset widget.
        class AudioOffsetWidget : public QWidget
        {
            Q_OBJECT

        public:
            AudioOffsetWidget(App*, QWidget* parent = nullptr);

            virtual ~AudioOffsetWidget();

        private Q_SLOTS:
            void _offsetCallback(double);

        private:
            void _playersUpdate(const QVector<QSharedPointer<qt::TimelinePlayer> >&);
            void _offsetUpdate();

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
