// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2022 Darby Johnston
// All rights reserved.

#pragma once

#include <tlrQt/TimelineThumbnailProvider.h>

#include <tlrCore/IRender.h>
#include <tlrCore/TimelinePlayer.h>

#include <QAbstractTableModel>

namespace tlr
{
    //! Files model item.
    struct FilesModelItem
    {
        file::Path path;
        file::Path audioPath;

        bool init = false;

        otime::RationalTime duration = time::invalidTime;
        otime::RationalTime globalStartTime = time::invalidTime;
        avio::Info avInfo;

        double speed = 0.0;
        timeline::Playback playback = timeline::Playback::Stop;
        timeline::Loop loop = timeline::Loop::Loop;
        otime::RationalTime currentTime = time::invalidTime;
        otime::TimeRange inOutRange = time::invalidTimeRange;

        uint16_t videoLayer = 0;

        float volume = 0.F;
        bool mute = false;
        double audioOffset = 0.0;

        render::ImageOptions imageOptions;
    };

    //! Files model.
    class FilesModel : public std::enable_shared_from_this<FilesModel>
    {
        TLR_NON_COPYABLE(FilesModel);

    protected:
        void _init(const std::shared_ptr<core::Context>&);
        FilesModel();

    public:
        ~FilesModel();

        //! Create a new files model.
        static std::shared_ptr<FilesModel> create(const std::shared_ptr<core::Context>&);

        //! Observe the files.
        std::shared_ptr<observer::IList<std::shared_ptr<FilesModelItem> > > observeFiles() const;

        //! Observe the A file.
        std::shared_ptr<observer::IValue<std::shared_ptr<FilesModelItem> > > observeA() const;

        //! Observe the B files.
        std::shared_ptr<observer::IList<std::shared_ptr<FilesModelItem> > > observeB() const;

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

        //! Set the A file to the first file.
        void first();

        //! Set the A file to the list file.
        void last();

        //! Set the A file to the next file.
        void next();

        //! Set the A file to the previous file.
        void prev();

        //! Observe the layers.
        std::shared_ptr<observer::IList<int> > observeLayers() const;

        //! Set a layer.
        void setLayer(const std::shared_ptr<FilesModelItem>&, int layer);

        //! Set the A file layer to the next layer.
        void nextLayer();

        //! Set the A file layer to the previous layer.
        void prevLayer();

        //! Observe the image options.
        std::shared_ptr<observer::IList<render::ImageOptions> > observeImageOptions() const;

        //! Set the image options.
        void setImageOptions(const render::ImageOptions&);

        //! Observe the compare options.
        std::shared_ptr<observer::IValue<render::CompareOptions> > observeCompareOptions() const;

        //! Set the compare options.
        void setCompareOptions(const render::CompareOptions&);

    private:
        int _index(const std::shared_ptr<FilesModelItem>&) const;
        std::vector<int> _bIndexes() const;
        std::vector<std::shared_ptr<FilesModelItem> > _getActive() const;
        std::vector<int> _getLayers() const;
        std::vector<render::ImageOptions> _getImageOptions() const;

        std::weak_ptr<core::Context> _context;
        std::shared_ptr<observer::List<std::shared_ptr<FilesModelItem> > > _files;
        std::shared_ptr<observer::Value<std::shared_ptr<FilesModelItem> > > _a;
        std::shared_ptr<observer::List<std::shared_ptr<FilesModelItem> > > _b;
        std::shared_ptr<observer::List<std::shared_ptr<FilesModelItem> > > _active;
        std::shared_ptr<observer::List<int> > _layers;
        std::shared_ptr<observer::List<render::ImageOptions> > _imageOptions;
        std::shared_ptr<observer::Value<render::CompareOptions> > _compareOptions;
    };

    //! Base class for files table models.
    class FilesTableModel : public QAbstractTableModel
    {
        Q_OBJECT

    public:
        FilesTableModel(
            const std::shared_ptr<FilesModel>&,
            const std::shared_ptr<core::Context>&,
            QObject* parent = nullptr);

        //! Get the files.
        const std::vector<std::shared_ptr<FilesModelItem> >& files() const;

        int rowCount(const QModelIndex& parent = QModelIndex()) const override;
        int columnCount(const QModelIndex& parent = QModelIndex()) const override;
        Qt::ItemFlags flags(const QModelIndex&) const override;
        QVariant data(const QModelIndex&, int role = Qt::DisplayRole) const override;
        bool setData(const QModelIndex& index, const QVariant& value, int role = Qt::EditRole) override;
        QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;

    private Q_SLOTS:
        void _thumbailCallback(const QList<QPair<otime::RationalTime, QImage> >&);

    protected:
        int _index(const std::shared_ptr<FilesModelItem>&) const;

        std::weak_ptr<core::Context> _context;
        std::shared_ptr<FilesModel> _filesModel;
        std::vector<std::shared_ptr<FilesModelItem> > _files;
        std::vector<std::shared_ptr<FilesModelItem> > _active;
        std::shared_ptr<observer::ListObserver<std::shared_ptr<FilesModelItem> > > _filesObserver;
        std::shared_ptr<observer::ListObserver<std::shared_ptr<FilesModelItem> > > _activeObserver;
        std::shared_ptr<observer::ListObserver<int> > _layersObserver;
        std::map<std::shared_ptr<FilesModelItem>, QImage> _thumbnails;
        std::map<std::shared_ptr<FilesModelItem>, qt::TimelineThumbnailProvider*> _thumbnailProviders;
    };

    //! Files A model.
    class FilesAModel : public FilesTableModel
    {
        Q_OBJECT

    public:
        FilesAModel(
            const std::shared_ptr<FilesModel>&,
            const std::shared_ptr<core::Context>&,
            QObject* parent = nullptr);

        QVariant data(const QModelIndex&, int role = Qt::DisplayRole) const override;

    private:
        std::shared_ptr<FilesModelItem> _a;
        std::shared_ptr<observer::ValueObserver<std::shared_ptr<FilesModelItem> > > _aObserver;
    };

    //! Files B model.
    class FilesBModel : public FilesTableModel
    {
        Q_OBJECT

    public:
        FilesBModel(
            const std::shared_ptr<FilesModel>&,
            const std::shared_ptr<core::Context>&,
            QObject* parent = nullptr);

        QVariant data(const QModelIndex&, int role = Qt::DisplayRole) const override;
    private:
        std::vector<int> _bIndexes() const;

        std::vector<std::shared_ptr<FilesModelItem> > _b;
        std::shared_ptr<observer::ListObserver<std::shared_ptr<FilesModelItem> > > _bObserver;
    };
}
