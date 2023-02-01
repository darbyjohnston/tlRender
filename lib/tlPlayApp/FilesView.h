// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2023 Darby Johnston
// All rights reserved.

#pragma once

#include <tlPlayApp/FilesModel.h>

#include <QStyledItemDelegate>

namespace tl
{
    namespace play
    {
        //! Files layers item delegate.
        class FilesLayersItemDelegate : public QStyledItemDelegate
        {
            Q_OBJECT

        public:
            FilesLayersItemDelegate(QObject* parent = nullptr);

            QWidget* createEditor(QWidget* parent, const QStyleOptionViewItem&, const QModelIndex&) const override;
            void setEditorData(QWidget* editor, const QModelIndex&) const override;
            void setModelData(QWidget* editor, QAbstractItemModel*, const QModelIndex&) const override;
        };
    }
}
