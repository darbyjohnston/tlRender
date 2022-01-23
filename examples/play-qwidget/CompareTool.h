// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2022 Darby Johnston
// All rights reserved.

#pragma once

#include <QComboBox>

namespace tlr
{
    //! Compare tool.
    class CompareTool : public QWidget
    {
        Q_OBJECT

    public:
        CompareTool(QWidget* parent = nullptr);

    public Q_SLOTS:
        void setOptions(const tlr::render::CompareOptions&);

    Q_SIGNALS:
        void optionsChanged(const tlr::render::CompareOptions&);

    private:
        void _optionsUpdate();

        render::CompareOptions _options;

        QComboBox* _modeComboBox = nullptr;
    };
}
