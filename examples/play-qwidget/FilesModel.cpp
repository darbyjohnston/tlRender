// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2022 Darby Johnston
// All rights reserved.

#include "FilesModel.h"

#include <tlrCore/StringFormat.h>

#include <QApplication>
#include <QPalette>

namespace tlr
{
    FilesModel::FilesModel(QObject* parent) :
        QAbstractTableModel(parent)
    {}

    const std::vector<std::shared_ptr<FilesModelItem> >& FilesModel::items() const
    {
        return _items;
    }

    const std::shared_ptr<FilesModelItem>& FilesModel::a() const
    {
        return _a;
    }

    const std::vector<std::shared_ptr<FilesModelItem> >& FilesModel::b() const
    {
        return _b;
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
            case 0: out |= Qt::ItemIsUserCheckable; break;
            case 1: out |= Qt::ItemIsUserCheckable; break;
            case 3: out |= Qt::ItemIsEditable; break;
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
                case 2:
                    s = item->path.get(-1, false);
                    break;
                case 3:
                    if (!item->avInfo.video.empty() &&
                        item->videoLayer < item->avInfo.video.size())
                    {
                        s = item->avInfo.video[item->videoLayer].name;
                    }
                    break;
                case 4:
                    s = string::Format("{0}").arg(item->avInfo.audio);
                    break;
                }
                out.setValue(QString::fromUtf8(s.c_str()));
                break;
            }
            case Qt::EditRole:
                switch (index.column())
                {
                case 3: out.setValue(item->videoLayer); break;
                }
                break;
            case Qt::CheckStateRole:
                switch (index.column())
                {
                case 0: out.setValue(index.row() == aIndex ? Qt::Checked : Qt::Unchecked); break;
                case 1:
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
                case 3:
                    setLayer(index.row(), value.toInt());
                    out = true;
                    break;
                }
                break;
            case Qt::CheckStateRole:
                switch (index.column())
                {
                case 0:
                    if (value.toBool())
                    {
                        setA(index.row());
                        out = true;
                    }
                    break;
                case 1:
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
                case 0: out = tr("A"); break;
                case 1: out = tr("B"); break;
                case 2: out = tr("File"); break;
                case 3: out = tr("Layer"); break;
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
        Q_EMIT aChanged(_a);
        Q_EMIT dataChanged(
            this->index(aIndex, 0),
            this->index(aIndex, 0),
            { Qt::CheckStateRole });
        Q_EMIT dataChanged(
            this->index(index, 0),
            this->index(index, 0),
            { Qt::CheckStateRole });
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
                Q_EMIT aChanged(_a);
                Q_EMIT dataChanged(
                    this->index(aIndex, 0),
                    this->index(aIndex, 0),
                    { Qt::CheckStateRole });
                Q_EMIT dataChanged(
                    this->index(aNewIndex, 0),
                    this->index(aNewIndex, 0),
                    { Qt::CheckStateRole });

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
                if (_b.size() != bSize)
                {
                    Q_EMIT bChanged(_b);
                }
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
            Q_EMIT aChanged(_a);

            _b.clear();
            Q_EMIT bChanged(_b);
        }
    }

    void FilesModel::setA(int index)
    {
        const int aIndex = _index(_a);
        if (index >= 0 && index < _items.size() && index != aIndex)
        {
            _a = _items[index];
            Q_EMIT aChanged(_a);
            Q_EMIT dataChanged(
                this->index(aIndex, 0),
                this->index(aIndex, 0),
                { Qt::CheckStateRole });
            Q_EMIT dataChanged(
                this->index(index, 0),
                this->index(index, 0),
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
                Q_EMIT dataChanged(
                    this->index(index, 1),
                    this->index(index, 1),
                    { Qt::CheckStateRole });
            }
            else if (!value && i != bIndexes.end())
            {
                _b.erase(_b.begin() + (i - bIndexes.begin()));
                Q_EMIT dataChanged(
                    this->index(index, 1),
                    this->index(index, 1),
                    { Qt::CheckStateRole });
            }
        }
    }

    void FilesModel::first()
    {
        const int aIndex = _index(_a);
        if (!_items.empty() && aIndex != 0)
        {
            _a = _items[0];
            Q_EMIT aChanged(_a);
            Q_EMIT dataChanged(
                index(aIndex, 0),
                index(aIndex, 0),
                { Qt::CheckStateRole });
            Q_EMIT dataChanged(
                index(0, 0),
                index(0, 0),
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
            Q_EMIT aChanged(_a);
            Q_EMIT dataChanged(
                this->index(index, 0),
                this->index(index, 0),
                { Qt::CheckStateRole });
            Q_EMIT dataChanged(
                this->index(aIndex, 0),
                this->index(aIndex, 0),
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
            Q_EMIT aChanged(_a);
            Q_EMIT dataChanged(
                this->index(aIndex, 0),
                this->index(aIndex, 0),
                { Qt::CheckStateRole });
            Q_EMIT dataChanged(
                this->index(aNewIndex, 0),
                this->index(aNewIndex, 0),
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
            Q_EMIT aChanged(_a);
            Q_EMIT dataChanged(
                this->index(aIndex, 0),
                this->index(aIndex, 0),
                { Qt::CheckStateRole });
            Q_EMIT dataChanged(
                this->index(aNewIndex, 0),
                this->index(aNewIndex, 0),
                { Qt::CheckStateRole });
        }
    }

    void FilesModel::setLayer(int index, int layer)
    {
        if (index >= 0 &&
            index < _items.size() &&
            layer >= 0 &&
            layer < _items[index]->avInfo.video.size() &&
            layer != _items[index]->videoLayer)
        {
            _items[index]->videoLayer = layer;
            Q_EMIT layerChanged(index, layer);
        }
    }

    void FilesModel::setImageOptions(int index, const render::ImageOptions& imageOptions)
    {
        if (index >= 0 &&
            index < _items.size() &&
            imageOptions != _items[index]->imageOptions)
        {
            _items[index]->imageOptions = imageOptions;
            Q_EMIT imageOptionsChanged(index, imageOptions);
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
}
