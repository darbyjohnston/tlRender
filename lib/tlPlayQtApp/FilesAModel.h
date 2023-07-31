// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2023 Darby Johnston
// All rights reserved.

#pragma once

#include <tlPlayQtApp/IFilesTableModel.h>

namespace tl
{
    namespace play_qt
    {
        //! Files "A" model.
        class FilesAModel : public IFilesTableModel
        {
            Q_OBJECT

        public:
            FilesAModel(
                const std::shared_ptr<play::FilesModel>&,
                qt::TimelineThumbnailObject*,
                const std::shared_ptr<system::Context>&,
                QObject* parent = nullptr);

            virtual ~FilesAModel();

            int columnCount(const QModelIndex& parent = QModelIndex()) const override;
            Qt::ItemFlags flags(const QModelIndex&) const override;
            QVariant data(const QModelIndex&, int role = Qt::DisplayRole) const override;
            bool setData(const QModelIndex&, const QVariant&, int role) override;
            QVariant headerData(int section, Qt::Orientation, int role) const override;

        private:
            TLRENDER_PRIVATE();
        };
    }
}
