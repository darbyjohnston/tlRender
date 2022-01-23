// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2022 Darby Johnston
// All rights reserved.

#pragma once

#include <tlrCore/IRender.h>

#include <QComboBox>
#include <QToolBox>

namespace tlr
{
    //! YUV range widget.
    class YUVRangeWidget : public QWidget
    {
        Q_OBJECT

    public:
        YUVRangeWidget(QWidget* parent = nullptr);

    public Q_SLOTS:
        void setValue(tlr::render::YUVRange);

    Q_SIGNALS:
        void valueChanged(tlr::render::YUVRange);

    private Q_SLOTS:
        void _callback(int);

    private:
        void _valueUpdate();

        render::YUVRange _value = tlr::render::YUVRange::First;

        QComboBox* _comboBox = nullptr;
    };

    //! Image tool.
    class ImageTool : public QToolBox
    {
        Q_OBJECT

    public:
        ImageTool(QWidget* parent = nullptr);

    public Q_SLOTS:
        void setOptions(const tlr::render::ImageOptions&);

    Q_SIGNALS:
        void optionsChanged(const tlr::render::ImageOptions&);

    private Q_SLOTS:
        void _yuvRangeCallback(tlr::render::YUVRange);
        void _currentItemCallback(int);

    private:
        void _optionsUpdate();

        render::ImageOptions _options;

        YUVRangeWidget* _yuvRangeWidget = nullptr;
    };
}
