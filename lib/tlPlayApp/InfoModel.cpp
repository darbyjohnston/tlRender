// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2022 Darby Johnston
// All rights reserved.

#include <tlPlayApp/InfoModel.h>

#include <tlIO/IO.h>

#include <tlCore/String.h>
#include <tlCore/StringFormat.h>

namespace tl
{
    namespace play
    {
        struct VideoInfoModel::Private
        {
            io::Info info;
            QList<QPair<QString, QString> > items;
        };

        VideoInfoModel::VideoInfoModel(QObject* parent) :
            QAbstractTableModel(parent),
            _p(new Private)
        {}

        VideoInfoModel::~VideoInfoModel()
        {}

        void VideoInfoModel::setInfo(const io::Info& value)
        {
            TLRENDER_P();
            if (value == p.info)
                return;
            p.info = value;
            beginResetModel();
            p.items.clear();
            if (!value.video.empty())
            {
                {
                    std::stringstream ss;
                    ss << p.info.video[0].size;
                    p.items.push_back(QPair<QString, QString>("resolution", QString::fromUtf8(ss.str().c_str())));
                }
                {
                    std::stringstream ss;
                    ss.precision(2);
                    ss << std::fixed;
                    ss << p.info.video[0].pixelAspectRatio;
                    p.items.push_back(QPair<QString, QString>("Pixel aspect ratio", QString::fromUtf8(ss.str().c_str())));
                }
                {
                    std::stringstream ss;
                    ss << p.info.video[0].pixelType;
                    p.items.push_back(QPair<QString, QString>("Pixel type", QString::fromUtf8(ss.str().c_str())));
                }
                {
                    std::stringstream ss;
                    ss << p.info.video[0].yuvRange;
                    p.items.push_back(QPair<QString, QString>("YUV range", QString::fromUtf8(ss.str().c_str())));
                }
                {
                    std::stringstream ss;
                    ss << string::getLabel(p.info.video[0].layout.mirror.x);
                    p.items.push_back(QPair<QString, QString>("Mirror x", QString::fromUtf8(ss.str().c_str())));
                }
                {
                    std::stringstream ss;
                    ss << string::getLabel(p.info.video[0].layout.mirror.y);
                    p.items.push_back(QPair<QString, QString>("Mirror y", QString::fromUtf8(ss.str().c_str())));
                }
                {
                    std::stringstream ss;
                    ss << static_cast<int>(p.info.video[0].layout.alignment);
                    p.items.push_back(QPair<QString, QString>("Alignment", QString::fromUtf8(ss.str().c_str())));
                }
                {
                    std::stringstream ss;
                    ss << p.info.video[0].layout.endian;
                    p.items.push_back(QPair<QString, QString>("Endian", QString::fromUtf8(ss.str().c_str())));
                }
            }
            endResetModel();
        }

        int VideoInfoModel::rowCount(const QModelIndex& parent) const
        {
            return _p->items.count();
        }

        int VideoInfoModel::columnCount(const QModelIndex& parent) const
        {
            return 2;
        }

        QVariant VideoInfoModel::data(const QModelIndex& index, int role) const
        {
            TLRENDER_P();
            QVariant out;
            if (index.isValid() &&
                index.row() >= 0 &&
                index.row() < p.items.count() &&
                index.column() >= 0 &&
                index.column() < 2)
            {
                switch (role)
                {
                case Qt::DisplayRole:
                {
                    switch (index.column())
                    {
                    case 0: out.setValue(p.items.at(index.row()).first); break;
                    case 1: out.setValue(p.items.at(index.row()).second); break;
                    }
                    break;
                }
                case Qt::ToolTipRole:
                    out.setValue(QString("%1: %2").arg(p.items.at(index.row()).first).arg(p.items.at(index.row()).second));
                    break;
                default: break;
                }
            }
            return out;
        }

        QVariant VideoInfoModel::headerData(int section, Qt::Orientation orientation, int role) const
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

        struct AudioInfoModel::Private
        {
            io::Info info;
            QList<QPair<QString, QString> > items;
        };

        AudioInfoModel::AudioInfoModel(QObject* parent) :
            QAbstractTableModel(parent),
            _p(new Private)
        {}

        AudioInfoModel::~AudioInfoModel()
        {}

        void AudioInfoModel::setInfo(const io::Info& value)
        {
            TLRENDER_P();
            if (value == p.info)
                return;
            p.info = value;
            beginResetModel();
            p.items.clear();
            {
                std::stringstream ss;
                ss << static_cast<int>(p.info.audio.channelCount);
                p.items.push_back(QPair<QString, QString>("Channels", QString::fromUtf8(ss.str().c_str())));
            }
            {
                std::stringstream ss;
                ss << p.info.audio.dataType;
                p.items.push_back(QPair<QString, QString>("Type", QString::fromUtf8(ss.str().c_str())));
            }
            {
                std::stringstream ss;
                ss << p.info.audio.sampleRate;
                p.items.push_back(QPair<QString, QString>("Sample rate", QString::fromUtf8(ss.str().c_str())));
            }
            endResetModel();
        }

        int AudioInfoModel::rowCount(const QModelIndex& parent) const
        {
            return _p->items.count();
        }

        int AudioInfoModel::columnCount(const QModelIndex& parent) const
        {
            return 2;
        }

        QVariant AudioInfoModel::data(const QModelIndex& index, int role) const
        {
            TLRENDER_P();
            QVariant out;
            if (index.isValid() &&
                index.row() >= 0 &&
                index.row() < p.items.count() &&
                index.column() >= 0 &&
                index.column() < 2)
            {
                switch (role)
                {
                case Qt::DisplayRole:
                {
                    switch (index.column())
                    {
                    case 0: out.setValue(p.items.at(index.row()).first); break;
                    case 1: out.setValue(p.items.at(index.row()).second); break;
                    }
                    break;
                }
                case Qt::ToolTipRole:
                    out.setValue(QString("%1: %2").arg(p.items.at(index.row()).first).arg(p.items.at(index.row()).second));
                    break;
                default: break;
                }
            }
            return out;
        }

        QVariant AudioInfoModel::headerData(int section, Qt::Orientation orientation, int role) const
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

        struct TagsModel::Private
        {
            std::map<std::string, std::string> tags;
            QList<QPair<QString, QString> > items;
        };

        TagsModel::TagsModel(QObject* parent) :
            QAbstractTableModel(parent),
            _p(new Private)
        {}

        TagsModel::~TagsModel()
        {}

        void TagsModel::setTags(const std::map<std::string, std::string>& value)
        {
            TLRENDER_P();
            if (value == p.tags)
                return;
            p.tags = value;
            beginResetModel();
            p.items.clear();
            for (const auto& i : p.tags)
            {
                p.items.push_back(QPair<QString, QString>(
                    QString::fromUtf8(i.first.c_str()),
                    QString::fromUtf8(i.second.c_str())));
            }
            endResetModel();
        }

        int TagsModel::rowCount(const QModelIndex& parent) const
        {
            return _p->items.count();
        }

        int TagsModel::columnCount(const QModelIndex& parent) const
        {
            return 2;
        }

        QVariant TagsModel::data(const QModelIndex& index, int role) const
        {
            TLRENDER_P();
            QVariant out;
            if (index.isValid() &&
                index.row() >= 0 &&
                index.row() < p.items.count() &&
                index.column() >= 0 &&
                index.column() < 2)
            {
                switch (role)
                {
                case Qt::DisplayRole:
                {
                    switch (index.column())
                    {
                    case 0: out.setValue(p.items.at(index.row()).first); break;
                    case 1: out.setValue(p.items.at(index.row()).second); break;
                    }
                    break;
                }
                case Qt::ToolTipRole:
                    out.setValue(QString("%1: %2").arg(p.items.at(index.row()).first).arg(p.items.at(index.row()).second));
                    break;
                default: break;
                }
            }
            return out;
        }

        QVariant TagsModel::headerData(int section, Qt::Orientation orientation, int role) const
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
}
