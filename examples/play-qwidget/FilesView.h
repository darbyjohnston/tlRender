// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2022 Darby Johnston
// All rights reserved.

#pragma once

#include "FilesModel.h"

#include <QSignalMapper>
#include <QStyledItemDelegate>

namespace tlr
{
    //! Files tool.
    class FilesLayersItemDelegate : public QStyledItemDelegate
    {
        Q_OBJECT

    public:
        FilesLayersItemDelegate(QObject* parent = nullptr);

        QWidget* createEditor(QWidget* parent, const QStyleOptionViewItem&, const QModelIndex&) const override;
        void setEditorData(QWidget* editor, const QModelIndex&) const override;
        void setModelData(QWidget* editor, QAbstractItemModel*, const QModelIndex&) const override;

    private:
        QSignalMapper* _mapper;
    };
}
