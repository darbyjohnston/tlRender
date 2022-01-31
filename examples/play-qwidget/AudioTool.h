// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2022 Darby Johnston
// All rights reserved.

#pragma once

#include "ToolWidget.h"

#include <QDoubleSpinBox>
#include <QSlider>

namespace tlr
{
    //! Audio offset widget.
    class AudioOffsetWidget : public QWidget
    {
        Q_OBJECT

    public:
        AudioOffsetWidget(QWidget* parent = nullptr);

    public Q_SLOTS:
        void setAudioOffset(double);

    Q_SIGNALS:
        void offsetChanged(double);

    private Q_SLOTS:
        void _sliderCallback(int);
        void _spinBoxCallback(double);
        void _resetCallback();

    private:
        void _offsetUpdate();

        double _offset = 0.0;
        QDoubleSpinBox* _spinBox = nullptr;
        QSlider* _slider = nullptr;
    };

    //! Audio tool.
    class AudioTool : public ToolWidget
    {
        Q_OBJECT

    public:
        AudioTool(QWidget* parent = nullptr);

    public Q_SLOTS:
        void setAudioOffset(double);

    Q_SIGNALS:
        void audioOffsetChanged(double);

    private:
        AudioOffsetWidget* _offsetWidget = nullptr;
    };
}
