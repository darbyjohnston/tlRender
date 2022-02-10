// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2022 Darby Johnston
// All rights reserved.

#pragma once

#include "ColorModel.h"
#include "ToolWidget.h"

#include <QLineEdit>
#include <QComboBox>

namespace tlr
{
    //! Color tool.
    class ColorTool : public ToolWidget
    {
        Q_OBJECT

    public:
        ColorTool(
            const std::shared_ptr<ColorModel>& colorModel,
            QWidget* parent = nullptr);

        ~ColorTool() override;

    private:
        void _widgetUpdate();

        std::shared_ptr<ColorModel> _colorModel;
        ColorModelData _data;
        QLineEdit* _fileNameLineEdit = nullptr;
        QComboBox* _inputComboBox = nullptr;
        QComboBox* _displayComboBox = nullptr;
        QComboBox* _viewComboBox = nullptr;
        std::shared_ptr<observer::ValueObserver<ColorModelData> > _dataObserver;
    };
}
