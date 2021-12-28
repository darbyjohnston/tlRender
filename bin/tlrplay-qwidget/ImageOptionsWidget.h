// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021 Darby Johnston
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

    Q_SIGNALS:
        void yuvRangeChanged(tlr::render::YUVRange);

    private Q_SLOTS:
        void _yuvRangeCallback(int);

    private:
        QComboBox* _yuvRangeComboBox = nullptr;
    };

    //! Image options widget.
    class ImageOptionsWidget : public QToolBox
    {
        Q_OBJECT

    public:
        ImageOptionsWidget(QWidget* parent = nullptr);

    Q_SIGNALS:
        void imageOptionsChanged(const tlr::render::ImageOptions&);

    private Q_SLOTS:
        void _yuvRangeCallback(tlr::render::YUVRange);
        void _currentItemCallback(int);

    private:
        render::ImageOptions _imageOptions;
    };
}
