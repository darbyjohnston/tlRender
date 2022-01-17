// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2022 Darby Johnston
// All rights reserved.

#pragma once

#include <tlrCore/Image.h>

#include <QAbstractListModel>

namespace tlr
{
    //! Layers model.
    class LayersModel : public QAbstractListModel
    {
        Q_OBJECT

    public:
        LayersModel(QObject* parent = nullptr);

        void set(const std::vector<imaging::Info>&, int current);

        int current() const;
        void setCurrent(int);

        int rowCount(const QModelIndex& parent = QModelIndex()) const override;
        QVariant data(const QModelIndex&, int role = Qt::DisplayRole) const override;

    public Q_SLOTS:
        void first();
        void last();
        void next();
        void prev();

    Q_SIGNALS:
        void countChanged(int);
        void currentChanged(int);

    private:
        std::vector<imaging::Info> _items;
        int _current = -1;
    };
}
