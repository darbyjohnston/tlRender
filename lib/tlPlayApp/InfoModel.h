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

    namespace play
    {
        //! Video information model.
        class VideoInfoModel : public QAbstractTableModel
        {
            Q_OBJECT

        public:
            VideoInfoModel(QObject* parent = nullptr);

            ~VideoInfoModel() override;

            //! Set the information.
            void setInfo(const io::Info&);

            int rowCount(const QModelIndex& parent = QModelIndex()) const override;
            int columnCount(const QModelIndex& parent = QModelIndex()) const override;
            QVariant data(const QModelIndex&, int role = Qt::DisplayRole) const override;
            QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;

        private:
            TLRENDER_PRIVATE();
        };

        //! Audio information model.
        class AudioInfoModel : public QAbstractTableModel
        {
            Q_OBJECT

        public:
            AudioInfoModel(QObject* parent = nullptr);

            ~AudioInfoModel() override;

            //! Set the information.
            void setInfo(const io::Info&);

            int rowCount(const QModelIndex& parent = QModelIndex()) const override;
            int columnCount(const QModelIndex& parent = QModelIndex()) const override;
            QVariant data(const QModelIndex&, int role = Qt::DisplayRole) const override;
            QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;

        private:
            TLRENDER_PRIVATE();
        };

        //! Tags model.
        class TagsModel : public QAbstractTableModel
        {
            Q_OBJECT

        public:
            TagsModel(QObject* parent = nullptr);

            ~TagsModel() override;

            //! Set the tags.
            void setTags(const std::map<std::string, std::string>&);

            int rowCount(const QModelIndex& parent = QModelIndex()) const override;
            int columnCount(const QModelIndex& parent = QModelIndex()) const override;
            QVariant data(const QModelIndex&, int role = Qt::DisplayRole) const override;
            QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;

        private:
            TLRENDER_PRIVATE();
        };
    }
}
