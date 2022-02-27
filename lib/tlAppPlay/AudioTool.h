// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2022 Darby Johnston
// All rights reserved.

#pragma once

#include <tlAppPlay/ToolWidget.h>

namespace tl
{
    namespace play
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
        class AudioTool : public ToolWidget
        {
            Q_OBJECT

        public:
            AudioTool(QWidget* parent = nullptr);

            ~AudioTool() override;

        public Q_SLOTS:
            void setAudioOffset(double);

        Q_SIGNALS:
            void audioOffsetChanged(double);

        private:
            TLRENDER_PRIVATE();
        };
    }
}
