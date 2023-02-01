// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2023 Darby Johnston
// All rights reserved.

#pragma once

#include <tlPlayApp/FilesModel.h>

#include <QAbstractTableModel>

namespace tl
{
    namespace qt
    {
        class TimelineThumbnailProvider;
    }

    namespace play
    {
        class FilesModel;

        //! Base class for files table models.
        class FilesTableModel : public QAbstractTableModel
        {
            Q_OBJECT

        public:
            FilesTableModel(
                const std::shared_ptr<FilesModel>&,
                qt::TimelineThumbnailProvider*,
                const std::shared_ptr<system::Context>&,
                QObject* parent = nullptr);

            virtual ~FilesTableModel() = 0;

            //! Get the files.
            const std::vector<std::shared_ptr<FilesModelItem> >& files() const;

            int rowCount(const QModelIndex& parent = QModelIndex()) const override;
            Qt::ItemFlags flags(const QModelIndex&) const override;
            QVariant data(const QModelIndex&, int role = Qt::DisplayRole) const override;

        private Q_SLOTS:
            void _thumbnailsCallback(qint64, const QList<QPair<otime::RationalTime, QImage> >&);

        protected:
            int _index(const std::shared_ptr<FilesModelItem>&) const;

            std::shared_ptr<FilesModel> _filesModel;
            std::vector<std::shared_ptr<FilesModelItem> > _files;

        private:
            TLRENDER_PRIVATE();
        };
    }
}
