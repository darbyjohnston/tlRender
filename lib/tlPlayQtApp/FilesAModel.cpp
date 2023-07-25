// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2023 Darby Johnston
// All rights reserved.

#include <tlPlayQtApp/FilesAModel.h>

#include <QApplication>
#include <QPalette>

namespace tl
{
    namespace play_qt
    {
        struct FilesAModel::Private
        {
            std::shared_ptr<play::FilesModelItem> a;
            std::vector<std::shared_ptr<play::FilesModelItem> > active;
            std::shared_ptr<observer::ValueObserver<std::shared_ptr<play::FilesModelItem> > > aObserver;
            std::shared_ptr<observer::ListObserver<std::shared_ptr<play::FilesModelItem> > > activeObserver;
            std::shared_ptr<observer::ListObserver<int> > layersObserver;
        };

        FilesAModel::FilesAModel(
            const std::shared_ptr<play::FilesModel>& filesModel,
            qt::TimelineThumbnailObject* thumbnailObject,
            const std::shared_ptr<system::Context>& context,
            QObject* parent) :
            IFilesTableModel(filesModel, thumbnailObject, context, parent),
            _p(new Private)
        {
            TLRENDER_P();

            p.aObserver = observer::ValueObserver<std::shared_ptr<play::FilesModelItem> >::create(
                filesModel->observeA(),
                [this](const std::shared_ptr<play::FilesModelItem>& value)
                {
                    const int prevIndex = _index(_p->a);
                    _p->a = value;
                    const int index = _index(_p->a);
                    Q_EMIT dataChanged(
                        this->index(index, 0),
                        this->index(index, 1),
                        { Qt::BackgroundRole, Qt::ForegroundRole });
                    Q_EMIT dataChanged(
                        this->index(prevIndex, 0),
                        this->index(prevIndex, 1),
                        { Qt::BackgroundRole, Qt::ForegroundRole });
                });

            p.activeObserver = observer::ListObserver<std::shared_ptr<play::FilesModelItem> >::create(
                filesModel->observeActive(),
                [this](const std::vector<std::shared_ptr<play::FilesModelItem> >& value)
                {
                    _p->active = value;
                });

            p.layersObserver = observer::ListObserver<int>::create(
                filesModel->observeLayers(),
                [this](const std::vector<int>& value)
                {
                    for (size_t i = 0; i < value.size(); ++i)
                    {
                        Q_EMIT dataChanged(
                            this->index(value[i], 1),
                            this->index(value[i], 1),
                            { Qt::DisplayRole, Qt::EditRole });
                    }
                });
        }

        FilesAModel::~FilesAModel()
        {}

        int FilesAModel::columnCount(const QModelIndex & parent) const
        {
            return 2;
        }
        
        Qt::ItemFlags FilesAModel::flags(const QModelIndex& index) const
        {
            TLRENDER_P();
            Qt::ItemFlags out = IFilesTableModel::flags(index);
            if (index.isValid() &&
                index.row() >= 0 &&
                index.row() < _files.size() &&
                index.column() >= 0 &&
                index.column() < 2)
            {
                switch (index.column())
                {
                case 1: out |= Qt::ItemIsEditable; break;
                }
            }
            return out;
        }

        QVariant FilesAModel::data(const QModelIndex& index, int role) const
        {
            TLRENDER_P();
            QVariant out = IFilesTableModel::data(index, role);
            if (index.isValid() &&
                index.row() >= 0 &&
                index.row() < _files.size() &&
                index.column() >= 0 &&
                index.column() < 2)
            {
                const auto& item = _files[index.row()];
                switch (role)
                {
                case Qt::DisplayRole:
                {
                    switch (index.column())
                    {
                    case 1:
                        if (!item->videoLayers.empty() &&
                            item->videoLayer < item->videoLayers.size())
                        {
                            const std::string s = item->videoLayers[item->videoLayer];
                            out.setValue(QString::fromUtf8(s.c_str()));
                        }
                        break;
                    }
                    break;
                }
                case Qt::EditRole:
                    switch (index.column())
                    {
                    case 1: out.setValue(item->videoLayer); break;
                    }
                    break;
                case Qt::BackgroundRole:
                {
                    const int aIndex = _index(p.a);
                    if (aIndex == index.row())
                    {
                        out.setValue(QBrush(qApp->palette().color(QPalette::ColorRole::Highlight)));
                    }
                    break;
                }
                case Qt::ForegroundRole:
                {
                    const int aIndex = _index(p.a);
                    if (aIndex == index.row())
                    {
                        out.setValue(QBrush(qApp->palette().color(QPalette::ColorRole::HighlightedText)));
                    }
                    break;
                }
                default: break;
                }
            }
            return out;
        }

        bool FilesAModel::setData(const QModelIndex& index, const QVariant& value, int role)
        {
            TLRENDER_P();
            bool out = false;
            if (index.isValid() &&
                index.row() >= 0 &&
                index.row() < _files.size() &&
                index.column() >= 0 &&
                index.column() < 2)
            {
                const auto& item = _files[index.row()];
                switch (role)
                {
                case Qt::EditRole:
                    switch (index.column())
                    {
                    case 1:
                        _filesModel->setLayer(item, value.toInt());
                        out = true;
                        break;
                    }
                    break;
                default: break;
                }
            }
            return out;
        }

        QVariant FilesAModel::headerData(int section, Qt::Orientation orientation, int role) const
        {
            QVariant out;
            if (Qt::Horizontal == orientation)
            {
                switch (role)
                {
                case Qt::DisplayRole:
                    switch (section)
                    {
                    case 0: out = tr("Name"); break;
                    case 1: out = tr("Layer"); break;
                    }
                    break;
                default: break;
                }
            }
            return out;
        }
    }
}
