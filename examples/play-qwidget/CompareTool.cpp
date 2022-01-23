// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2022 Darby Johnston
// All rights reserved.

#include "App.h"
#include "CompareTool.h"

#include <QBoxLayout>
#include <QFormLayout>
#include <QSignalBlocker>

namespace tlr
{
    CompareTool::CompareTool(QWidget* parent) :
        QWidget(parent)
    {
        _modeComboBox = new QComboBox;
        for (const auto& i : render::getCompareModeLabels())
        {
            _modeComboBox->addItem(QString::fromUtf8(i.c_str()));
        }

        auto layout = new QVBoxLayout;
        layout->addWidget(_modeComboBox);
        setLayout(layout);

        _optionsUpdate();
    }

    void CompareTool::setOptions(const render::CompareOptions& options)
    {
        if (options == _options)
            return;
        _options = options;
        _optionsUpdate();
    }

    void CompareTool::_optionsUpdate()
    {
    }
}
