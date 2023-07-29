// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2023 Darby Johnston
// All rights reserved.

#pragma once

#include <tlPlayQtApp/IToolWidget.h>

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
            AudioOffsetWidget(QWidget* parent = nullptr);

            ~AudioOffsetWidget() override;

        public Q_SLOTS:
            void setAudioOffset(double);

        Q_SIGNALS:
            void audioOffsetChanged(double);

        private:
            void _offsetUpdate();

            TLRENDER_PRIVATE();
        };

        //! Audio tool.
        class AudioTool : public IToolWidget
        {
            Q_OBJECT

        public:
            AudioTool(App*, QWidget* parent = nullptr);

            ~AudioTool() override;

        public Q_SLOTS:
            void setAudioOffset(double);

        Q_SIGNALS:
            void audioOffsetChanged(double);

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
