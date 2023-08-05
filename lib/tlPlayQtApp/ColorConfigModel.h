// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2023 Darby Johnston
// All rights reserved.

#pragma once

#include <tlPlay/ColorConfigModel.h>

#include <QAbstractListModel>

namespace tl
{
    namespace play_qt
    {
        //! Color input list model.
        class ColorInputListModel : public QAbstractListModel
        {
            Q_OBJECT

        public:
            ColorInputListModel(
                const std::shared_ptr<play::ColorConfigModel>&,
                QObject* parent = nullptr);

            virtual ~ColorInputListModel();

            int rowCount(const QModelIndex& parent = QModelIndex()) const override;
            QVariant data(const QModelIndex&, int role = Qt::DisplayRole) const override;

        protected:
            TLRENDER_PRIVATE();
        };

        //! Color display list model.
        class ColorDisplayListModel : public QAbstractListModel
        {
            Q_OBJECT

        public:
            ColorDisplayListModel(
                const std::shared_ptr<play::ColorConfigModel>&,
                QObject* parent = nullptr);

            virtual ~ColorDisplayListModel();

            int rowCount(const QModelIndex& parent = QModelIndex()) const override;
            QVariant data(const QModelIndex&, int role = Qt::DisplayRole) const override;

        protected:
            TLRENDER_PRIVATE();
        };

        //! Color view list model.
        class ColorViewListModel : public QAbstractListModel
        {
            Q_OBJECT

        public:
            ColorViewListModel(
                const std::shared_ptr<play::ColorConfigModel>&,
                QObject* parent = nullptr);

            virtual ~ColorViewListModel();

            int rowCount(const QModelIndex& parent = QModelIndex()) const override;
            QVariant data(const QModelIndex&, int role = Qt::DisplayRole) const override;

        protected:
            TLRENDER_PRIVATE();
        };
    }
}
