// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2022 Darby Johnston
// All rights reserved.

#pragma once

#include "ToolWidget.h"

namespace tlr
{
    //! Information tool.
    class InfoTool : public ToolWidget
    {
        Q_OBJECT

    public:
        InfoTool(QWidget* parent = nullptr);

        ~InfoTool() override;
    };
}
