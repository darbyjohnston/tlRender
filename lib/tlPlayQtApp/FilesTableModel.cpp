// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2023 Darby Johnston
// All rights reserved.

#include <tlPlayQtApp/FilesTableModel.h>

#include <tlQt/TimelineThumbnailObject.h>

namespace tl
{
    namespace play_qt
    {
        struct FilesTableModel::Private
        {
            std::weak_ptr<system::Context> context;
            qt::TimelineThumbnailObject* thumbnailObject = nullptr;
            std::map<qint64, std::shared_ptr<play::FilesModelItem> > thumbnailRequestIds;
            std::map<std::shared_ptr<play::FilesModelItem>, QImage> thumbnails;
            std::shared_ptr<observer::ListObserver<std::shared_ptr<play::FilesModelItem> > > filesObserver;
        };

        FilesTableModel::FilesTableModel(
            const std::shared_ptr<play::FilesModel>& filesModel,
            qt::TimelineThumbnailObject* thumbnailObject,
            const std::shared_ptr<system::Context>& context,
            QObject* parent) :
            QAbstractTableModel(parent),
            _filesModel(filesModel),
            _p(new Private)
        {
            TLRENDER_P();

            p.context = context;
            p.thumbnailObject = thumbnailObject;

            p.filesObserver = observer::ListObserver<std::shared_ptr<play::FilesModelItem> >::create(
                filesModel->observeFiles(),
                [this](const std::vector<std::shared_ptr<play::FilesModelItem> >& value)
                {
                    beginResetModel();
                    _files = value;
                    if (_p->thumbnailObject)
                    {
                        for (auto i : _p->thumbnailRequestIds)
                        {
                            _p->thumbnailObject->cancelRequests(i.first);
                        }
                        _p->thumbnailRequestIds.clear();
                        if (auto context = _p->context.lock())
                        {
                            for (auto i : _files)
                            {
                                try
                                {
                                    qint64 id = _p->thumbnailObject->request(
                                        QString::fromUtf8(i->path.get().c_str()),
                                        QSize(120, 80));
                                    _p->thumbnailRequestIds[id] = i;
                                }
                                catch (const std::exception&)
                                {
                                }
                            }
                        }
                    }
                    endResetModel();
                });

            if (p.thumbnailObject)
            {
                connect(
                    p.thumbnailObject,
                    SIGNAL(thumbails(qint64, const QList<QPair<otime::RationalTime, QImage> >&)),
                    SLOT(_thumbnailsCallback(qint64, const QList<QPair<otime::RationalTime, QImage> >&)));
            }
        }

        FilesTableModel::~FilesTableModel()
        {}

        const std::vector<std::shared_ptr<play::FilesModelItem> >& FilesTableModel::files() const
        {
            return _files;
        }

        int FilesTableModel::rowCount(const QModelIndex&) const
        {
            return _files.size();
        }

        Qt::ItemFlags FilesTableModel::flags(const QModelIndex& index) const
        {
            TLRENDER_P();
            Qt::ItemFlags out = Qt::NoItemFlags;
            if (index.isValid() &&
                index.row() >= 0 &&
                index.row() < _files.size())
            {
                out |= Qt::ItemIsEnabled;
                out |= Qt::ItemIsSelectable;
            }
            return out;
        }

        QVariant FilesTableModel::data(const QModelIndex& index, int role) const
        {
            TLRENDER_P();
            QVariant out;
            if (index.isValid() &&
                index.row() >= 0 &&
                index.row() < _files.size())
            {
                const auto& item = _files[index.row()];
                switch (role)
                {
                case Qt::DisplayRole:
                {
                    switch (index.column())
                    {
                    case 0:
                    {
                        const std::string s = item->path.get(-1, false);
                        out.setValue(QString::fromUtf8(s.c_str()));
                        break;
                    }
                    }
                    break;
                }
                case Qt::DecorationRole:
                    switch (index.column())
                    {
                    case 0:
                    {
                        const auto i = p.thumbnails.find(item);
                        if (i != p.thumbnails.end())
                        {
                            out.setValue(i->second);
                        }
                        break;
                    }
                    }
                    break;
                case Qt::ToolTipRole:
                    out.setValue(QString::fromUtf8(item->path.get().c_str()));
                    break;
                default: break;
                }
            }
            return out;
        }

        void FilesTableModel::_thumbnailsCallback(qint64 id, const QList<QPair<otime::RationalTime, QImage> >& value)
        {
            TLRENDER_P();
            if (!value.isEmpty())
            {
                auto i = p.thumbnailRequestIds.find(id);
                if (i != p.thumbnailRequestIds.end())
                {
                    p.thumbnails[i->second] = value[0].second;
                    const auto j = std::find(_files.begin(), _files.end(), i->second);
                    if (j != _files.end())
                    {
                        const int index = j - _files.begin();
                        Q_EMIT dataChanged(
                            this->index(index, 0),
                            this->index(index, 0),
                            { Qt::DecorationRole });
                    }
                    p.thumbnailRequestIds.erase(i);
                }
            }
        }

        int FilesTableModel::_index(const std::shared_ptr<play::FilesModelItem>& item) const
        {
            TLRENDER_P();
            int out = -1;
            const auto i = std::find(_files.begin(), _files.end(), item);
            if (i != _files.end())
            {
                out = i - _files.begin();
            }
            return out;
        }
    }
}
