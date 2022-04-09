// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2022 Darby Johnston
// All rights reserved.

#pragma once

#include <tlTimeline/IRender.h>
#include <tlTimeline/TimelinePlayer.h>

#include <QAbstractTableModel>

namespace tl
{
    namespace qt
    {
        class TimelineThumbnailProvider;
    }

    namespace play
    {
        //! Files model item.
        struct FilesModelItem
        {
            file::Path path;
            file::Path audioPath;

            bool init = false;

            otime::RationalTime duration = time::invalidTime;
            otime::RationalTime globalStartTime = time::invalidTime;
            io::Info ioInfo;

            double speed = 0.0;
            timeline::Playback playback = timeline::Playback::Stop;
            timeline::Loop loop = timeline::Loop::Loop;
            otime::RationalTime currentTime = time::invalidTime;
            otime::TimeRange inOutRange = time::invalidTimeRange;

            uint16_t videoLayer = 0;

            float volume = 0.F;
            bool mute = false;
            double audioOffset = 0.0;
        };

        //! Files model.
        class FilesModel : public std::enable_shared_from_this<FilesModel>
        {
            TLRENDER_NON_COPYABLE(FilesModel);

        protected:
            void _init(const std::shared_ptr<system::Context>&);
            FilesModel();

        public:
            ~FilesModel();

            //! Create a new files model.
            static std::shared_ptr<FilesModel> create(const std::shared_ptr<system::Context>&);

            //! Observe the files.
            std::shared_ptr<observer::IList<std::shared_ptr<FilesModelItem> > > observeFiles() const;

            //! Observe the A file.
            std::shared_ptr<observer::IValue<std::shared_ptr<FilesModelItem> > > observeA() const;

            //! Observe the A file index.
            std::shared_ptr<observer::IValue<int> > observeAIndex() const;

            //! Observe the B files.
            std::shared_ptr<observer::IList<std::shared_ptr<FilesModelItem> > > observeB() const;

            //! Observe the B file indexes.
            std::shared_ptr<observer::IList<int> > observeBIndexes() const;

            //! Observe the active files.
            std::shared_ptr<observer::IList<std::shared_ptr<FilesModelItem> > > observeActive() const;

            //! Add a file.
            void add(const std::shared_ptr<FilesModelItem>&);

            //! Close the current A file.
            void close();

            //! Close all the files.
            void closeAll();

            //! Set the A file.
            void setA(int index);

            //! Set the B files.
            void setB(int index, bool);

            //! Toggle a B file.
            void toggleB(int index);

            //! Clear the B files.
            void clearB();

            //! Set the A file to the first file.
            void first();

            //! Set the A file to the list file.
            void last();

            //! Set the A file to the next file.
            void next();

            //! Set the A file to the previous file.
            void prev();

            //! Set the A file to the first file.
            void firstB();

            //! Set the A file to the list file.
            void lastB();

            //! Set the A file to the next file.
            void nextB();

            //! Set the A file to the previous file.
            void prevB();

            //! Observe the layers.
            std::shared_ptr<observer::IList<int> > observeLayers() const;

            //! Set a layer.
            void setLayer(const std::shared_ptr<FilesModelItem>&, int layer);

            //! Set the A file layer to the next layer.
            void nextLayer();

            //! Set the A file layer to the previous layer.
            void prevLayer();

            //! Observe the compare options.
            std::shared_ptr<observer::IValue<timeline::CompareOptions> > observeCompareOptions() const;

            //! Set the compare options.
            void setCompareOptions(const timeline::CompareOptions&);

        private:
            int _index(const std::shared_ptr<FilesModelItem>&) const;
            std::vector<int> _bIndexes() const;
            std::vector<std::shared_ptr<FilesModelItem> > _getActive() const;
            std::vector<int> _getLayers() const;

            TLRENDER_PRIVATE();
        };

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

            ~FilesTableModel() override;

            //! Get the files.
            const std::vector<std::shared_ptr<FilesModelItem> >& files() const;

            int rowCount(const QModelIndex& parent = QModelIndex()) const override;
            int columnCount(const QModelIndex& parent = QModelIndex()) const override;
            Qt::ItemFlags flags(const QModelIndex&) const override;
            QVariant data(const QModelIndex&, int role = Qt::DisplayRole) const override;
            bool setData(const QModelIndex& index, const QVariant& value, int role = Qt::EditRole) override;
            QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;

        private Q_SLOTS:
            void _thumbnailsCallback(qint64, const QList<QPair<otime::RationalTime, QImage> >&);

        protected:
            int _index(const std::shared_ptr<FilesModelItem>&) const;

            std::vector<std::shared_ptr<FilesModelItem> > _files;

        private:
            TLRENDER_PRIVATE();
        };

        //! Files A model.
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

        //! Files B model.
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
