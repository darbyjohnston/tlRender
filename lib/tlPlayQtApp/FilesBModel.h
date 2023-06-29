// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2023 Darby Johnston
// All rights reserved.

#pragma once

#include <tlPlayQtApp/FilesTableModel.h>

namespace tl
{
    namespace play_qt
    {
        //! Files "B" model.
        class FilesBModel : public FilesTableModel
        {
            Q_OBJECT

        public:
            FilesBModel(
                const std::shared_ptr<play::FilesModel>&,
                qt::TimelineThumbnailObject*,
                const std::shared_ptr<system::Context>&,
                QObject* parent = nullptr);

            ~FilesBModel() override;

            int columnCount(const QModelIndex& parent = QModelIndex()) const override;
            Qt::ItemFlags flags(const QModelIndex&) const override;
            QVariant data(const QModelIndex&, int role = Qt::DisplayRole) const override;
            bool setData(const QModelIndex&, const QVariant&, int role) override;
            QVariant headerData(int section, Qt::Orientation, int role) const override;

        private:
            std::vector<int> _bIndexes() const;

            TLRENDER_PRIVATE();
        };
    }
}
