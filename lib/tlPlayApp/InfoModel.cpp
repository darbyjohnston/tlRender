// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2022 Darby Johnston
// All rights reserved.

#include <tlPlayApp/InfoModel.h>

#include <tlIO/IO.h>

#include <tlCore/StringFormat.h>

namespace tl
{
    namespace play
    {
        struct InfoModel::Private
        {
            io::Info info;
            QList<QPair<QString, QString> > items;
        };

        InfoModel::InfoModel(QObject* parent) :
            QAbstractTableModel(parent),
            _p(new Private)
        {}

        InfoModel::~InfoModel()
        {}

        void InfoModel::setInfo(const io::Info& value)
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
                    p.items.push_back(QPair<QString, QString>("Video Resolution", QString::fromUtf8(ss.str().c_str())));
                }
                {
                    std::stringstream ss;
                    ss.precision(2);
                    ss << std::fixed;
                    ss << p.info.video[0].pixelAspectRatio;
                    p.items.push_back(QPair<QString, QString>("Video Pixel Aspect Ratio", QString::fromUtf8(ss.str().c_str())));
                }
                {
                    std::stringstream ss;
                    ss << p.info.video[0].pixelType;
                    p.items.push_back(QPair<QString, QString>("Video Pixel Type", QString::fromUtf8(ss.str().c_str())));
                }
                {
                    std::stringstream ss;
                    ss << p.info.video[0].yuvRange;
                    p.items.push_back(QPair<QString, QString>("Video YUV Range", QString::fromUtf8(ss.str().c_str())));
                }
                {
                    std::stringstream ss;
                    ss << p.info.video[0].layout.mirror.x;
                    p.items.push_back(QPair<QString, QString>("Video Mirror X", QString::fromUtf8(ss.str().c_str())));
                }
                {
                    std::stringstream ss;
                    ss << p.info.video[0].layout.mirror.y;
                    p.items.push_back(QPair<QString, QString>("Video Mirror Y", QString::fromUtf8(ss.str().c_str())));
                }
                {
                    std::stringstream ss;
                    ss << static_cast<int>(p.info.video[0].layout.alignment);
                    p.items.push_back(QPair<QString, QString>("Video Alignment", QString::fromUtf8(ss.str().c_str())));
                }
                {
                    std::stringstream ss;
                    ss << p.info.video[0].layout.endian;
                    p.items.push_back(QPair<QString, QString>("Video Endian", QString::fromUtf8(ss.str().c_str())));
                }
                {
                    std::stringstream ss;
                    ss << p.info.videoTime;
                    p.items.push_back(QPair<QString, QString>("Video Time", QString::fromUtf8(ss.str().c_str())));
                }
                {
                    std::stringstream ss;
                    ss << static_cast<int>(p.info.audio.channelCount);
                    p.items.push_back(QPair<QString, QString>("Audio Channels", QString::fromUtf8(ss.str().c_str())));
                }
                {
                    std::stringstream ss;
                    ss << p.info.audio.dataType;
                    p.items.push_back(QPair<QString, QString>("Audio Type", QString::fromUtf8(ss.str().c_str())));
                }
                {
                    std::stringstream ss;
                    ss << p.info.audio.sampleRate;
                    p.items.push_back(QPair<QString, QString>("Audio Sample Rate", QString::fromUtf8(ss.str().c_str())));
                }
                {
                    std::stringstream ss;
                    ss << p.info.audioTime;
                    p.items.push_back(QPair<QString, QString>("Audio Time", QString::fromUtf8(ss.str().c_str())));
                }
                for (const auto& i : p.info.tags)
                {
                    p.items.push_back(QPair<QString, QString>(
                        QString::fromUtf8(i.first.c_str()),
                        QString::fromUtf8(i.second.c_str())));
                }
            }
            endResetModel();
        }

        int InfoModel::rowCount(const QModelIndex& parent) const
        {
            return _p->items.count();
        }

        int InfoModel::columnCount(const QModelIndex& parent) const
        {
            return 2;
        }

        QVariant InfoModel::data(const QModelIndex& index, int role) const
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
}
