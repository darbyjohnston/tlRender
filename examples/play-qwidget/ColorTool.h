// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2022 Darby Johnston
// All rights reserved.

#pragma once

#include "ToolWidget.h"

namespace tlr
{
    //! Color tool.
    class ColorTool : public ToolWidget
    {
        Q_OBJECT

    public:
        ColorTool(QWidget* parent = nullptr);

        ~ColorTool() override;
    };
}
