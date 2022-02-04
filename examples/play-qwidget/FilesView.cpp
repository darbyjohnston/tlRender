// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2022 Darby Johnston
// All rights reserved.

#include "FilesView.h"

#include <QComboBox>

namespace tlr
{
    FilesLayersItemDelegate::FilesLayersItemDelegate(QObject* parent) :
        QStyledItemDelegate(parent),
        _mapper(new QSignalMapper(this))
    {
        connect(
            _mapper,
            SIGNAL(mappedObject(QObject*)),
            SLOT(_mapperCallback(QObject*)));
    }

    QWidget* FilesLayersItemDelegate::createEditor(QWidget* parent, const QStyleOptionViewItem& options, const QModelIndex& index) const
    {
        QComboBox* out = new QComboBox(parent);
        if (auto model = qobject_cast<const FilesModel*>(index.model()))
        {
            const auto& items = model->items();
            if (index.isValid() &&
                index.row() >= 0 &&
                index.row() < items.size())
            {
                for (const auto& video : items[index.row()]->avInfo.video)
                {
                    out->addItem(QString::fromUtf8(video.name.c_str()));
                }
            }
        }
        connect(
            out,
            SIGNAL(activated(int)),
            _mapper,
            SLOT(map()));
        _mapper->setMapping(out, out);
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

    void FilesLayersItemDelegate::_mapperCallback(QObject* value)
    {
        Q_EMIT commitData(qobject_cast<QWidget*>(value));
    }
}
