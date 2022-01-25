// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2022 Darby Johnston
// All rights reserved.

#include "FilesModel.h"

#include <tlrCore/StringFormat.h>

#include <QApplication>
#include <QPalette>

namespace tlr
{
    FilesModel::FilesModel(
        const std::shared_ptr<core::Context>& context,
        QObject* parent) :
        QAbstractTableModel(parent),
        _context(context)
    {}

    const std::vector<std::shared_ptr<FilesModelItem> >& FilesModel::items() const
    {
        return _items;
    }

    int FilesModel::rowCount(const QModelIndex&) const
    {
        return _items.size();
    }

    int FilesModel::columnCount(const QModelIndex& parent) const
    {
        return 4;
    }

    Qt::ItemFlags FilesModel::flags(const QModelIndex& index) const
    {
        Qt::ItemFlags out = Qt::NoItemFlags;
        if (index.isValid() &&
            index.row() >= 0 &&
            index.row() < _items.size() &&
            index.column() >= 0 &&
            index.column() < 4)
        {
            out |= Qt::ItemIsEnabled;
            out |= Qt::ItemIsSelectable;
            switch (index.column())
            {
            case 1: out |= Qt::ItemIsEditable; break;
            case 2: out |= Qt::ItemIsUserCheckable; break;
            case 3: out |= Qt::ItemIsUserCheckable; break;
            }
        }
        return out;
    }

    QVariant FilesModel::data(const QModelIndex& index, int role) const
    {
        QVariant out;
        if (index.isValid() &&
            index.row() >= 0 &&
            index.row() < _items.size() &&
            index.column() >= 0 &&
            index.column() < 4)
        {
            const auto& item = _items[index.row()];
            const int aIndex = _index(_a);
            switch (role)
            {
            case Qt::DisplayRole:
            {
                std::string s;
                switch (index.column())
                {
                case 0:
                    s = item->path.get(-1, false);
                    break;
                case 1:
                    if (!item->avInfo.video.empty() &&
                        item->videoLayer < item->avInfo.video.size())
                    {
                        s = item->avInfo.video[item->videoLayer].name;
                    }
                    break;
                }
                out.setValue(QString::fromUtf8(s.c_str()));
                break;
            }
            case Qt::DecorationRole:
                switch (index.column())
                {
                case 0:
                {
                    const auto i = _thumbnails.find(item);
                    if (i != _thumbnails.end())
                    {
                        out.setValue(i->second);
                    }
                    break;
                }
                }
                break;
            case Qt::EditRole:
                switch (index.column())
                {
                case 1: out.setValue(item->videoLayer); break;
                }
                break;
            case Qt::CheckStateRole:
                switch (index.column())
                {
                case 2: out.setValue(index.row() == aIndex ? Qt::Checked : Qt::Unchecked); break;
                case 3:
                {
                    const auto bIndexes = _bIndexes();
                    const auto i = std::find(bIndexes.begin(), bIndexes.end(), index.row());
                    out.setValue(i != bIndexes.end() ? Qt::Checked : Qt::Unchecked);
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

    bool FilesModel::setData(const QModelIndex& index, const QVariant& value, int role)
    {
        bool out = false;
        if (index.isValid() &&
            index.row() >= 0 &&
            index.row() < _items.size() &&
            index.column() >= 0 &&
            index.column() < 4)
        {
            const auto& item = _items[index.row()];
            const int aIndex = _index(_a);
            switch (role)
            {
            case Qt::EditRole:
                switch (index.column())
                {
                case 1:
                    setLayer(item, value.toInt());
                    out = true;
                    break;
                }
                break;
            case Qt::CheckStateRole:
                switch (index.column())
                {
                case 2:
                    if (value.toBool())
                    {
                        setA(index.row());
                        out = true;
                    }
                    break;
                case 3:
                    setB(index.row(), value == Qt::Checked);
                    out = true;
                    break;
                }
                break;
            default: break;
            }
        }
        return out;
    }

    QVariant FilesModel::headerData(int section, Qt::Orientation orientation, int role) const
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
                case 2: out = tr("A"); break;
                case 3: out = tr("B"); break;
                }
                break;
            default: break;
            }
        }
        return out;
    }

    void FilesModel::add(const std::shared_ptr<FilesModelItem>& item)
    {
        const int index = _items.size();
        beginInsertRows(QModelIndex(), index, index);
        _items.push_back(item);
        endInsertRows();
        Q_EMIT countChanged(_items.size());

        const int aIndex = _index(_a);
        _a = item;
        Q_EMIT activeChanged(_active());
        Q_EMIT dataChanged(
            this->index(aIndex, 2),
            this->index(aIndex, 2),
            { Qt::CheckStateRole });
        Q_EMIT dataChanged(
            this->index(index, 2),
            this->index(index, 2),
            { Qt::CheckStateRole });

        if (auto context = _context.lock())
        {
            auto timeline = timeline::Timeline::create(item->path.get(), context);
            _thumbnailProviders[item] = new qt::TimelineThumbnailProvider(timeline, context);
            connect(
                _thumbnailProviders[item],
                SIGNAL(thumbails(const QList<QPair<otime::RationalTime, QImage> >&)),
                SLOT(_thumbailCallback(const QList<QPair<otime::RationalTime, QImage> >&)));
            _thumbnailProviders[item]->request(timeline->getGlobalStartTime(), QSize(120, 80));
        }
    }

    void FilesModel::close()
    {
        if (!_items.empty() && _a)
        {
            const auto i = std::find(_items.begin(), _items.end(), _a);
            if (i != _items.end())
            {
                const int aIndex = i - _items.begin();
                beginRemoveRows(QModelIndex(), aIndex, aIndex);
                _items.erase(i);
                endRemoveRows();
                Q_EMIT countChanged(_items.size());

                const int aNewIndex = math::clamp(aIndex, 0, static_cast<int>(_items.size()) - 1);
                _a = aNewIndex != -1 ? _items[aNewIndex] : nullptr;

                const size_t bSize = _b.size();
                auto b = _b.begin();
                while (b != _b.end())
                {
                    const auto j = std::find(_items.begin(), _items.end(), *b);
                    if (j == _items.end())
                    {
                        b = _b.erase(b);
                    }
                    else
                    {
                        ++b;
                    }
                }

                Q_EMIT activeChanged(_active());
                Q_EMIT dataChanged(
                    this->index(aIndex, 2),
                    this->index(aIndex, 2),
                    { Qt::CheckStateRole });
                Q_EMIT dataChanged(
                    this->index(aNewIndex, 2),
                    this->index(aNewIndex, 2),
                    { Qt::CheckStateRole });
            }
        }
    }

    void FilesModel::closeAll()
    {
        if (!_items.empty())
        {
            beginRemoveRows(QModelIndex(), 0, _items.size() - 1);
            _items.clear();
            endRemoveRows();
            Q_EMIT countChanged(_items.size());

            _a = nullptr;
            _b.clear();
            Q_EMIT activeChanged(_active());
        }
    }

    void FilesModel::setA(int index)
    {
        const int aIndex = _index(_a);
        if (index >= 0 && index < _items.size() && index != aIndex)
        {
            _a = _items[index];
            Q_EMIT activeChanged(_active());
            Q_EMIT dataChanged(
                this->index(aIndex, 2),
                this->index(aIndex, 2),
                { Qt::CheckStateRole });
            Q_EMIT dataChanged(
                this->index(index, 2),
                this->index(index, 2),
                { Qt::CheckStateRole });
        }
    }

    void FilesModel::setB(int index, bool value)
    {
        if (index >= 0 && index < _items.size())
        {
            const auto bIndexes = _bIndexes();
            const auto i = std::find(bIndexes.begin(), bIndexes.end(), index);
            if (value && i == bIndexes.end())
            {
                _b.push_back(_items[index]);
            }
            else if (!value && i != bIndexes.end())
            {
                _b.erase(_b.begin() + (i - bIndexes.begin()));
            }
            Q_EMIT activeChanged(_active());
            Q_EMIT dataChanged(
                this->index(index, 3),
                this->index(index, 3),
                { Qt::CheckStateRole });
        }
    }

    void FilesModel::first()
    {
        const int aIndex = _index(_a);
        if (!_items.empty() && aIndex != 0)
        {
            _a = _items[0];
            Q_EMIT activeChanged(_active());
            Q_EMIT dataChanged(
                index(aIndex, 2),
                index(aIndex, 2),
                { Qt::CheckStateRole });
            Q_EMIT dataChanged(
                index(0, 2),
                index(0, 2),
                { Qt::CheckStateRole });
        }
    }

    void FilesModel::last()
    {
        const int index = static_cast<int>(_items.size()) - 1;
        const int aIndex = _index(_a);
        if (!_items.empty() && index != aIndex)
        {
            _a = _items[index];
            Q_EMIT activeChanged(_active());
            Q_EMIT dataChanged(
                this->index(index, 2),
                this->index(index, 2),
                { Qt::CheckStateRole });
            Q_EMIT dataChanged(
                this->index(aIndex, 2),
                this->index(aIndex, 2),
                { Qt::CheckStateRole });
        }
    }

    void FilesModel::next()
    {
        if (!_items.empty())
        {
            const int aIndex = _index(_a);
            int aNewIndex = aIndex + 1;
            if (aNewIndex >= _items.size())
            {
                aNewIndex = 0;
            }
            _a = _items[aNewIndex];
            Q_EMIT activeChanged(_active());
            Q_EMIT dataChanged(
                this->index(aIndex, 2),
                this->index(aIndex, 2),
                { Qt::CheckStateRole });
            Q_EMIT dataChanged(
                this->index(aNewIndex, 2),
                this->index(aNewIndex, 2),
                { Qt::CheckStateRole });
        }
    }

    void FilesModel::prev()
    {
        if (!_items.empty())
        {
            const int aIndex = _index(_a);
            int aNewIndex = aIndex - 1;
            if (aNewIndex < 0)
            {
                aNewIndex = _items.size() - 1;
            }
            _a = _items[aNewIndex];
            Q_EMIT activeChanged(_active());
            Q_EMIT dataChanged(
                this->index(aIndex, 2),
                this->index(aIndex, 2),
                { Qt::CheckStateRole });
            Q_EMIT dataChanged(
                this->index(aNewIndex, 2),
                this->index(aNewIndex, 2),
                { Qt::CheckStateRole });
        }
    }

    void FilesModel::setLayer(const std::shared_ptr<FilesModelItem>& item, int layer)
    {
        const int index = _index(item);
        if (index != -1 &&
            layer < _items[index]->avInfo.video.size() &&
            layer != _items[index]->videoLayer)
        {
            _items[index]->videoLayer = layer;
            Q_EMIT layerChanged(item, layer);
        }
    }

    void FilesModel::setImageOptions(const std::shared_ptr<FilesModelItem>& item, const render::ImageOptions& imageOptions)
    {
        const int index = _index(item);
        if (index != -1 &&
            imageOptions != _items[index]->imageOptions)
        {
            _items[index]->imageOptions = imageOptions;
            Q_EMIT imageOptionsChanged(item, imageOptions);
        }
    }

    void FilesModel::_thumbailCallback(const QList<QPair<otime::RationalTime, QImage> >& value)
    {
        if (!value.isEmpty())
        {
            for (auto i = _thumbnailProviders.begin(); i != _thumbnailProviders.end(); ++i)
            {
                if (i->second == sender())
                {
                    _thumbnails[i->first] = value[0].second;
                    const auto j = std::find(_items.begin(), _items.end(), i->first);
                    if (j != _items.end())
                    {
                        const int index = j - _items.begin();
                        Q_EMIT dataChanged(
                            this->index(index, 0),
                            this->index(index, 0),
                            { Qt::DecorationRole });
                    }
                    delete i->second;
                    _thumbnailProviders.erase(i);
                    break;
                }
            }
        }
    }

    int FilesModel::_index(const std::shared_ptr<FilesModelItem>& item) const
    {
        int out = -1;
        if (!_items.empty())
        {
            const auto i = std::find(_items.begin(), _items.end(), item);
            if (i != _items.end())
            {
                out = i - _items.begin();
            }
        }
        return out;
    }

    std::vector<int> FilesModel::_bIndexes() const
    {
        std::vector<int> out;
        for (const auto& b : _b)
        {
            out.push_back(_index(b));
        }
        return out;
    }

    std::vector<std::shared_ptr<FilesModelItem> > FilesModel::_active() const
    {
        std::vector<std::shared_ptr<FilesModelItem> > out;
        if (_a)
        {
            out.push_back(_a);
        }
        for (const auto& b : _b)
        {
            out.push_back(b);
        }
        return out;
    }
}
