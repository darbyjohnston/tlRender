// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2022 Darby Johnston
// All rights reserved.

#pragma once

#include <tlPlayApp/FilesTableModel.h>

namespace tl
{
    namespace play
    {
        //! Files "B" model.
        class FilesBModel : public FilesTableModel
        {
            Q_OBJECT

        public:
            FilesBModel(
                const std::shared_ptr<FilesModel>&,
                qt::TimelineThumbnailProvider*,
                const std::shared_ptr<system::Context>&,
                QObject* parent = nullptr);

            ~FilesBModel() override;

            QVariant data(const QModelIndex&, int role = Qt::DisplayRole) const override;

        private:
            std::vector<int> _bIndexes() const;

            TLRENDER_PRIVATE();
        };
    }
}
