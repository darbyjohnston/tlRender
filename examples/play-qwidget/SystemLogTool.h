// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2022 Darby Johnston
// All rights reserved.

#pragma once

#include "ToolWidget.h"

namespace tlr
{
    //! System log tool.
    class SystemLogTool : public ToolWidget
    {
        Q_OBJECT

    public:
        SystemLogTool(QWidget* parent = nullptr);

        ~SystemLogTool() override;
    };
}
