// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2023 Darby Johnston
// All rights reserved.

#pragma once

#include <tlPlay/OCIOModel.h>

#include <QAbstractListModel>

namespace tl
{
    namespace play_qt
    {
        //! OpenColorIO input list model.
        class OCIOInputListModel : public QAbstractListModel
        {
            Q_OBJECT

        public:
            OCIOInputListModel(
                const std::shared_ptr<play::OCIOModel>&,
                QObject* parent = nullptr);

            virtual ~OCIOInputListModel();

            int rowCount(const QModelIndex& parent = QModelIndex()) const override;
            QVariant data(const QModelIndex&, int role = Qt::DisplayRole) const override;

        protected:
            TLRENDER_PRIVATE();
        };

        //! OpenColorIO display list model.
        class OCIODisplayListModel : public QAbstractListModel
        {
            Q_OBJECT

        public:
            OCIODisplayListModel(
                const std::shared_ptr<play::OCIOModel>&,
                QObject* parent = nullptr);

            virtual ~OCIODisplayListModel();

            int rowCount(const QModelIndex& parent = QModelIndex()) const override;
            QVariant data(const QModelIndex&, int role = Qt::DisplayRole) const override;

        protected:
            TLRENDER_PRIVATE();
        };

        //! OpenColorIO view list model.
        class OCIOViewListModel : public QAbstractListModel
        {
            Q_OBJECT

        public:
            OCIOViewListModel(
                const std::shared_ptr<play::OCIOModel>&,
                QObject* parent = nullptr);

            virtual ~OCIOViewListModel();

            int rowCount(const QModelIndex& parent = QModelIndex()) const override;
            QVariant data(const QModelIndex&, int role = Qt::DisplayRole) const override;

        protected:
            TLRENDER_PRIVATE();
        };
    }
}
