// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2022 Darby Johnston
// All rights reserved.

#include "FilesModel.h"

#include <QIcon>

namespace tlr
{
    FilesModelItem::FilesModelItem()
    {}

    FilesModelItem::FilesModelItem(const std::string & fileName) :
        path(fileName)
    {}

    FilesModelItem::FilesModelItem(const std::string& fileName, const std::string& audioFileName) :
        path(fileName),
        audioPath(audioFileName)
    {}
    
    FilesModelItem::FilesModelItem(const std::shared_ptr<timeline::TimelinePlayer>& player) :
        path(player->getPath()),
        audioPath(player->getAudioPath()),
        init(true),
        duration(player->getDuration()),
        globalStartTime(player->getGlobalStartTime()),
        avInfo(player->getAVInfo()),
        speed(player->observeSpeed()->get()),
        playback(player->observePlayback()->get()),
        loop(player->observeLoop()->get()),
        currentTime(player->observeCurrentTime()->get()),
        inOutRange(player->observeInOutRange()->get()),
        videoLayer(player->observeVideoLayer()->get()),
        volume(player->observeVolume()->get()),
        mute(player->observeMute()->get()),
        audioOffset(player->observeAudioOffset()->get())
    {}

    FilesModel::FilesModel(
        const std::shared_ptr<core::Context>& context,
        QObject* parent) :
        QAbstractListModel(parent)
    {}

    void FilesModel::add(const FilesModelItem& timeline)
    {
        beginInsertRows(QModelIndex(), _items.size(), _items.size());
        _items.push_back(timeline);
        endInsertRows();
        _current = _items.size() - 1;
        Q_EMIT currentChanged(&_items[_current]);
        Q_EMIT dataChanged(
            index(_current, 0),
            index(_current, 0),
            { Qt::DecorationRole });
    }

    void FilesModel::remove()
    {
        if (_current != -1)
        {
            beginRemoveRows(QModelIndex(), _current, _current);
            _items.erase(_items.begin() + _current);
            endRemoveRows();
            _current = std::min(_current, static_cast<int>(_items.size()) - 1);
            Q_EMIT currentChanged(_current != -1 ? &_items[_current] : nullptr);
            if (_current != -1)
            {
                Q_EMIT dataChanged(
                    index(_current, 0),
                    index(_current, 0),
                    { Qt::DecorationRole });
            }
        }
    }

    void FilesModel::clear()
    {
        if (!_items.empty())
        {
            beginRemoveRows(QModelIndex(), 0, _items.size() - 1);
            _items.clear();
            endRemoveRows();
            _current = -1;
            Q_EMIT currentChanged(nullptr);
        }
    }

    const FilesModelItem* FilesModel::current() const
    {
        return _current != -1 ? &_items[_current] : nullptr;
    }

    void FilesModel::setCurrent(const QModelIndex& index)
    {
        if (index.isValid() &&
            index.row() >= 0 &&
            index.row() < _items.size() &&
            index.row() != _current)
        {
            _current = index.row();
            Q_EMIT currentChanged(&_items[_current]);
            Q_EMIT dataChanged(
                this->index(_current, 0),
                this->index(_current, 0),
                { Qt::DecorationRole });
        }
    }

    void FilesModel::update(const FilesModelItem& item)
    {
        if (_current != -1)
        {
            _items[_current] = item;
            Q_EMIT dataChanged(
                index(_current, 0),
                index(_current, 0),
                { Qt::DisplayRole });
        }
    }

    int FilesModel::rowCount(const QModelIndex&) const
    {
        return _items.size();
    }

    QVariant FilesModel::data(const QModelIndex& index, int role) const
    {
        QVariant out;
        if (index.isValid() &&
            index.row() >= 0 &&
            index.row() < _items.size())
        {
            switch (role)
            {
            case Qt::DisplayRole:
                out.setValue(QString::fromUtf8(_items[index.row()].path.get(-1, false).c_str()));
                break;
            case Qt::DecorationRole:
                out = QIcon(index.row() == _current ? ":/Icons/PlaybackForward.svg" : ":/Icons/Empty.svg");
                break;
            default: break;
            }
        }
        return out;
    }

    void FilesModel::first()
    {
        if (!_items.empty() && _current != 0)
        {
            _current = 0;
            Q_EMIT currentChanged(&_items[_current]);
            if (_current != -1)
            {
                Q_EMIT dataChanged(
                    index(_current, 0),
                    index(_current, 0),
                    { Qt::DecorationRole });
            }
        }
    }

    void FilesModel::last()
    {
        if (!_items.empty() && _current != _items.size() - 1)
        {
            _current = _items.size() - 1;
            Q_EMIT currentChanged(&_items[_current]);
            if (_current != -1)
            {
                Q_EMIT dataChanged(
                    index(_current, 0),
                    index(_current, 0),
                    { Qt::DecorationRole });
            }
        }
    }

    void FilesModel::next()
    {
        if (_items.size() > 1)
        {
            ++_current;
            if (_current >= _items.size())
            {
                _current = 0;
            }
            Q_EMIT currentChanged(&_items[_current]);
            if (_current != -1)
            {
                Q_EMIT dataChanged(
                    index(_current, 0),
                    index(_current, 0),
                    { Qt::DecorationRole });
            }
        }
    }

    void FilesModel::prev()
    {
        if (_items.size() > 1)
        {
            --_current;
            if (_current < 0)
            {
                _current = _items.size() - 1;
            }
            Q_EMIT currentChanged(&_items[_current]);
            if (_current != -1)
            {
                Q_EMIT dataChanged(
                    index(_current, 0),
                    index(_current, 0),
                    { Qt::DecorationRole });
            }
        }
    }
}
