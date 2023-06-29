// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2023 Darby Johnston
// All rights reserved.

#include <tlPlayQtApp/FilesView.h>

#include <tlPlayQtApp/FilesTableModel.h>

#include <QComboBox>

namespace tl
{
    namespace play_qt
    {
        FilesLayersItemDelegate::FilesLayersItemDelegate(QObject* parent) :
            QStyledItemDelegate(parent)
        {}

        QWidget* FilesLayersItemDelegate::createEditor(QWidget* parent, const QStyleOptionViewItem& options, const QModelIndex& index) const
        {
            QComboBox* out = new QComboBox(parent);
            if (auto model = qobject_cast<const FilesTableModel*>(index.model()))
            {
                const auto& files = model->files();
                if (index.isValid() &&
                    index.row() >= 0 &&
                    index.row() < files.size())
                {
                    for (const auto& video : files[index.row()]->ioInfo.video)
                    {
                        out->addItem(QString::fromUtf8(video.name.c_str()));
                    }
                }
            }
            connect(
                out,
                QOverload<int>::of(&QComboBox::activated),
                [this, out]
                {
                    //! \bug Is there a way to avoid this const cast?
                    Q_EMIT const_cast<FilesLayersItemDelegate*>(this)->commitData(out);
                });
            return out;
        }

        void FilesLayersItemDelegate::setEditorData(QWidget* editor, const QModelIndex& index) const
        {
            QComboBox* comboBox = qobject_cast<QComboBox*>(editor);
            comboBox->setCurrentIndex(index.data(Qt::EditRole).toInt());
        }

        void FilesLayersItemDelegate::setModelData(QWidget* editor, QAbstractItemModel* model, const QModelIndex& index) const
        {
            QComboBox* comboBox = qobject_cast<QComboBox*>(editor);
            model->setData(index, comboBox->currentIndex(), Qt::EditRole);
        }
    }
}
