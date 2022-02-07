// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2022 Darby Johnston
// All rights reserved.

#include "InfoModel.h"

#include <tlrCore/StringFormat.h>

namespace tlr
{
    InfoModel::InfoModel(QObject* parent) :
        QAbstractTableModel(parent)
    {}

    void InfoModel::setInfo(const avio::Info& value)
    {
        if (value == _info)
            return;
        _info = value;
        beginResetModel();
        _items.clear();
        if (!value.video.empty())
        {
            {
                std::stringstream ss;
                ss << _info.video[0].size;
                _items.push_back(QPair<QString, QString>("Video Resolution", QString::fromUtf8(ss.str().c_str())));
            }
            {
                std::stringstream ss;
                ss.precision(2);
                ss << std::fixed;
                ss << _info.video[0].pixelAspectRatio;
                _items.push_back(QPair<QString, QString>("Video Pixel Aspect Ratio", QString::fromUtf8(ss.str().c_str())));
            }
            {
                std::stringstream ss;
                ss << _info.video[0].pixelType;
                _items.push_back(QPair<QString, QString>("Video Pixel Type", QString::fromUtf8(ss.str().c_str())));
            }
            {
                std::stringstream ss;
                ss << _info.video[0].yuvRange;
                _items.push_back(QPair<QString, QString>("Video YUV Range", QString::fromUtf8(ss.str().c_str())));
            }
            {
                std::stringstream ss;
                ss << _info.video[0].layout.mirror.x;
                _items.push_back(QPair<QString, QString>("Video Mirror X", QString::fromUtf8(ss.str().c_str())));
            }
            {
                std::stringstream ss;
                ss << _info.video[0].layout.mirror.y;
                _items.push_back(QPair<QString, QString>("Video Mirror Y", QString::fromUtf8(ss.str().c_str())));
            }
            {
                std::stringstream ss;
                ss << static_cast<int>(_info.video[0].layout.alignment);
                _items.push_back(QPair<QString, QString>("Video Alignment", QString::fromUtf8(ss.str().c_str())));
            }
            {
                std::stringstream ss;
                ss << _info.video[0].layout.endian;
                _items.push_back(QPair<QString, QString>("Video Endian", QString::fromUtf8(ss.str().c_str())));
            }
            {
                std::stringstream ss;
                ss << _info.videoType;
                _items.push_back(QPair<QString, QString>("Video Type", QString::fromUtf8(ss.str().c_str())));
            }
            {
                std::stringstream ss;
                ss << _info.videoTime;
                _items.push_back(QPair<QString, QString>("Video Time", QString::fromUtf8(ss.str().c_str())));
            }
            {
                std::stringstream ss;
                ss << static_cast<int>(_info.audio.channelCount);
                _items.push_back(QPair<QString, QString>("Audio Channels", QString::fromUtf8(ss.str().c_str())));
            }
            {
                std::stringstream ss;
                ss << _info.audio.dataType;
                _items.push_back(QPair<QString, QString>("Audio Type", QString::fromUtf8(ss.str().c_str())));
            }
            {
                std::stringstream ss;
                ss << _info.audio.sampleRate;
                _items.push_back(QPair<QString, QString>("Audio Sample Rate", QString::fromUtf8(ss.str().c_str())));
            }
            {
                std::stringstream ss;
                ss << _info.audioTime;
                _items.push_back(QPair<QString, QString>("Audio Time", QString::fromUtf8(ss.str().c_str())));
            }
            for (const auto& i : _info.tags)
            {
                _items.push_back(QPair<QString, QString>(
                    QString::fromUtf8(i.first.c_str()),
                    QString::fromUtf8(i.second.c_str())));
            }
        }
        endResetModel();
    }

    int InfoModel::rowCount(const QModelIndex& parent) const
    {
        return _items.count();
    }

    int InfoModel::columnCount(const QModelIndex& parent) const
    {
        return 2;
    }

    QVariant InfoModel::data(const QModelIndex& index, int role) const
    {
        QVariant out;
        if (index.isValid() &&
            index.row() >= 0 &&
            index.row() < _items.count() &&
            index.column() >= 0 &&
            index.column() < 2)
        {
            switch (role)
            {
            case Qt::DisplayRole:
            {
                switch (index.column())
                {
                case 0: out.setValue(_items.at(index.row()).first); break;
                case 1: out.setValue(_items.at(index.row()).second); break;
                }
                break;
            }
            case Qt::ToolTipRole:
                out.setValue(QString("%1: %2").arg(_items.at(index.row()).first).arg(_items.at(index.row()).second));
                break;
            default: break;
            }
        }
        return out;
    }

    QVariant InfoModel::headerData(int section, Qt::Orientation orientation, int role) const
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
                case 1: out = tr("Value"); break;
                }
                break;
            default: break;
            }
        }
        return out;
    }
}
