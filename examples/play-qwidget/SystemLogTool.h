// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2022 Darby Johnston
// All rights reserved.

#pragma once

#include "ToolWidget.h"

#include <tlrCore/Context.h>

#include <QListWidget>
#include <QToolButton>

namespace tlr
{
    //! System log tool.
    class SystemLogTool : public ToolWidget
    {
        Q_OBJECT

    public:
        SystemLogTool(
            const std::shared_ptr<core::Context>&,
            QWidget* parent = nullptr);

    private:
        QListWidget* _listWidget = nullptr;
        QToolButton* _clearButton = nullptr;
        std::shared_ptr<observer::ValueObserver<core::LogItem> > _logObserver;
    };
}
