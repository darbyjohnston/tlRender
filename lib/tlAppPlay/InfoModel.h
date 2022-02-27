// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2022 Darby Johnston
// All rights reserved.

#pragma once

#include <tlCore/Util.h>

#include <QAbstractTableModel>

#include <memory>

namespace tl
{
    namespace io
    {
        struct Info;
    }

    namespace app
    {
        namespace play
        {
            //! Information model.
            class InfoModel : public QAbstractTableModel
            {
                Q_OBJECT

            public:
                InfoModel(QObject* parent = nullptr);

                ~InfoModel() override;

                //! Set the information.
                void setInfo(const io::Info&);

                int rowCount(const QModelIndex& parent = QModelIndex()) const override;
                int columnCount(const QModelIndex& parent = QModelIndex()) const override;
                QVariant data(const QModelIndex&, int role = Qt::DisplayRole) const override;
                QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;

            private:
                TLRENDER_PRIVATE();
            };
        }
    }
}
