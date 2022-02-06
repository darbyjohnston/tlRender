// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2022 Darby Johnston
// All rights reserved.

#pragma once

#include "ToolWidget.h"

namespace tlr
{
    //! Messages tool.
    class MessagesTool : public ToolWidget
    {
        Q_OBJECT

    public:
        MessagesTool(QWidget* parent = nullptr);

        ~MessagesTool() override;
    };
}
