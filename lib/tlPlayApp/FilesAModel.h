// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2022 Darby Johnston
// All rights reserved.

#pragma once

#include <tlPlayApp/FilesTableModel.h>

namespace tl
{
    namespace play
    {
        //! Files "A" model.
        class FilesAModel : public FilesTableModel
        {
            Q_OBJECT

        public:
            FilesAModel(
                const std::shared_ptr<FilesModel>&,
                qt::TimelineThumbnailProvider*,
                const std::shared_ptr<system::Context>&,
                QObject* parent = nullptr);

            ~FilesAModel() override;

            QVariant data(const QModelIndex&, int role = Qt::DisplayRole) const override;

        private:
            TLRENDER_PRIVATE();
        };
    }
}
