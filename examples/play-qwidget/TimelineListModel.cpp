// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2022 Darby Johnston
// All rights reserved.

#include "TimelineListModel.h"

namespace tlr
{
    TimelineListItem::TimelineListItem()
    {}
    
    TimelineListItem::TimelineListItem(const std::shared_ptr<timeline::TimelinePlayer>& player) :
        path(player->getPath()),
        audioPath(player->getAudioPath()),
        duration(player->getDuration()),
        globalStartTime(player->getGlobalStartTime()),
        avIOInfo(player->getAVInfo()),
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

    TimelineListModel::TimelineListModel(
        const std::shared_ptr<core::Context>& context,
        QObject* parent) :
        QAbstractListModel(parent)
    {
    }

    void TimelineListModel::add(const TimelineListItem& timeline)
    {
        beginInsertRows(QModelIndex(), _items.size(), _items.size());
        _items.push_back(timeline);
        endInsertRows();
    }

    void TimelineListModel::remove(int index)
    {
        if (index >= 0 && index < _items.size())
        {
            beginRemoveRows(QModelIndex(), index, index);
            _items.erase(_items.begin() + index);
            endRemoveRows();
        }
    }

    TimelineListItem TimelineListModel::get(int index) const
    {
        TimelineListItem out;
        if (index >= 0 && index < _items.size())
        {
            out = _items[index];
        }
        return out;
    }

    void TimelineListModel::set(int index, const TimelineListItem& item)
    {
        if (index >= 0 && index < _items.size())
        {
            _items[index] = item;
            Q_EMIT dataChanged(
                createIndex(index, 0),
                createIndex(index, 0),
                { Qt::DisplayRole });
        }
    }

    int TimelineListModel::rowCount(const QModelIndex&) const
    {
        return _items.size();
    }

    QVariant TimelineListModel::data(const QModelIndex& index, int role) const
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
            default: break;
            }
        }
        return out;
    }
}
