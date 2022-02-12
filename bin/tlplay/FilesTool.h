// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2022 Darby Johnston
// All rights reserved.

#pragma once

#include "FilesModel.h"
#include "ToolWidget.h"

#include <QTreeView>

namespace tl
{
    //! Files tool.
    class FilesTool : public ToolWidget
    {
        Q_OBJECT

    public:
        FilesTool(
            const std::shared_ptr<FilesModel>&,
            const std::shared_ptr<core::Context>&,
            QWidget* parent = nullptr);

        ~FilesTool() override;

    private Q_SLOTS:
        void _activatedCallback(const QModelIndex&);

    private:
        std::shared_ptr<FilesModel> _filesModel;
        FilesAModel* _filesAModel = nullptr;
        QTreeView* _treeView = nullptr;
    };
}
