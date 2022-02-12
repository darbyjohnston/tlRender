// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2022 Darby Johnston
// All rights reserved.

#pragma once

#include "InfoModel.h"
#include "ToolWidget.h"

#include <QTreeView>

namespace tl
{
    //! Information tool.
    class InfoTool : public ToolWidget
    {
        Q_OBJECT

    public:
        InfoTool(QWidget* parent = nullptr);

        ~InfoTool() override;

        void setInfo(const avio::Info&);

    private:
        InfoModel* _infoModel = nullptr;
        QTreeView* _treeView = nullptr;
    };
}
