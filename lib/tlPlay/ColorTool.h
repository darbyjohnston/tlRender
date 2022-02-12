// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2022 Darby Johnston
// All rights reserved.

#pragma once

#include <tlPlay/ColorModel.h>
#include <tlPlay/ToolWidget.h>

#include <QLineEdit>
#include <QListView>
#include <QComboBox>

namespace tl
{
    namespace play
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
            QListView* _inputListView = nullptr;
            QListView* _displayListView = nullptr;
            QListView* _viewListView = nullptr;
            std::shared_ptr<observer::ValueObserver<ColorModelData> > _dataObserver;
        };
    }
}
