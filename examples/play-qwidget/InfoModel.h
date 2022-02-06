// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2022 Darby Johnston
// All rights reserved.

#pragma once

#include <tlrCore/AVIO.h>

#include <QAbstractTableModel>

namespace tlr
{
    //! Information model.
    class InfoModel : public QAbstractTableModel
    {
        Q_OBJECT

    public:
        InfoModel(QObject* parent = nullptr);

        void setInfo(const avio::Info&);

        int rowCount(const QModelIndex& parent = QModelIndex()) const override;
        int columnCount(const QModelIndex& parent = QModelIndex()) const override;
        QVariant data(const QModelIndex&, int role = Qt::DisplayRole) const override;
        QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;

    private:
        avio::Info _info;
        QList<QPair<QString, QString> > _items;
    };
}
