// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2023 Darby Johnston
// All rights reserved.

#include <tlPlayQtApp/ColorConfigModel.h>

#include <QApplication>
#include <QPalette>

namespace tl
{
    namespace play_qt
    {
        struct ColorInputListModel::Private
        {
            std::vector<std::string> inputs;
            size_t inputIndex = 0;
            std::shared_ptr<observer::ValueObserver<play::ColorConfigModelData> > dataObserver;
        };

        ColorInputListModel::ColorInputListModel(
            const std::shared_ptr<play::ColorConfigModel>& colorConfigModel,
            QObject* parent) :
            QAbstractListModel(parent),
            _p(new Private)
        {
            TLRENDER_P();

            p.dataObserver = observer::ValueObserver<play::ColorConfigModelData>::create(
                colorConfigModel->observeData(),
                [this](const play::ColorConfigModelData& value)
                {
                    beginResetModel();
                    _p->inputs = value.inputs;
                    _p->inputIndex = value.inputIndex;
                    endResetModel();
                });
        }

        ColorInputListModel::~ColorInputListModel()
        {}

        int ColorInputListModel::rowCount(const QModelIndex& parent) const
        {
            return _p->inputs.size();
        }

        QVariant ColorInputListModel::data(const QModelIndex& index, int role) const
        {
            TLRENDER_P();
            QVariant out;
            if (index.isValid() &&
                index.row() >= 0 &&
                index.row() < p.inputs.size() &&
                index.column() >= 0 &&
                index.column() < 2)
            {
                switch (role)
                {
                case Qt::DisplayRole:
                    out.setValue(QString::fromUtf8(p.inputs[index.row()].c_str()));
                    break;
                case Qt::BackgroundRole:
                    if (index.row() == p.inputIndex)
                    {
                        out.setValue(
                            QBrush(qApp->palette().color(QPalette::ColorRole::Highlight)));
                    }
                    break;
                case Qt::ForegroundRole:
                    if (index.row() == p.inputIndex)
                    {
                        out.setValue(
                            QBrush(qApp->palette().color(QPalette::ColorRole::HighlightedText)));
                    }
                    break;
                default: break;
                }
            }
            return out;
        }

        struct ColorDisplayListModel::Private
        {
            std::vector<std::string> displays;
            size_t displayIndex = 0;
            std::shared_ptr<observer::ValueObserver<play::ColorConfigModelData> > dataObserver;
        };

        ColorDisplayListModel::ColorDisplayListModel(
            const std::shared_ptr<play::ColorConfigModel>& colorConfigModel,
            QObject* parent) :
            QAbstractListModel(parent),
            _p(new Private)
        {
            TLRENDER_P();

            p.dataObserver = observer::ValueObserver<play::ColorConfigModelData>::create(
                colorConfigModel->observeData(),
                [this](const play::ColorConfigModelData& value)
                {
                    beginResetModel();
                    _p->displays = value.displays;
                    _p->displayIndex = value.displayIndex;
                    endResetModel();
                });
        }

        ColorDisplayListModel::~ColorDisplayListModel()
        {}

        int ColorDisplayListModel::rowCount(const QModelIndex& parent) const
        {
            return _p->displays.size();
        }

        QVariant ColorDisplayListModel::data(const QModelIndex& index, int role) const
        {
            TLRENDER_P();
            QVariant out;
            if (index.isValid() &&
                index.row() >= 0 &&
                index.row() < p.displays.size() &&
                index.column() >= 0 &&
                index.column() < 2)
            {
                switch (role)
                {
                case Qt::DisplayRole:
                    out.setValue(QString::fromUtf8(p.displays[index.row()].c_str()));
                    break;
                case Qt::BackgroundRole:
                    if (index.row() == p.displayIndex)
                    {
                        out.setValue(
                            QBrush(qApp->palette().color(QPalette::ColorRole::Highlight)));
                    }
                    break;
                case Qt::ForegroundRole:
                    if (index.row() == p.displayIndex)
                    {
                        out.setValue(
                            QBrush(qApp->palette().color(QPalette::ColorRole::HighlightedText)));
                    }
                    break;
                default: break;
                }
            }
            return out;
        }

        struct ColorViewListModel::Private
        {
            std::vector<std::string> views;
            size_t viewIndex = 0;
            std::shared_ptr<observer::ValueObserver<play::ColorConfigModelData> > dataObserver;
        };

        ColorViewListModel::ColorViewListModel(
            const std::shared_ptr<play::ColorConfigModel>& colorConfigModel,
            QObject* parent) :
            QAbstractListModel(parent),
            _p(new Private)
        {
            TLRENDER_P();

            p.dataObserver = observer::ValueObserver<play::ColorConfigModelData>::create(
                colorConfigModel->observeData(),
                [this](const play::ColorConfigModelData& value)
                {
                    beginResetModel();
                    _p->views = value.views;
                    _p->viewIndex = value.viewIndex;
                    endResetModel();
                });
        }

        ColorViewListModel::~ColorViewListModel()
        {}

        int ColorViewListModel::rowCount(const QModelIndex& parent) const
        {
            return _p->views.size();
        }

        QVariant ColorViewListModel::data(const QModelIndex& index, int role) const
        {
            TLRENDER_P();
            QVariant out;
            if (index.isValid() &&
                index.row() >= 0 &&
                index.row() < p.views.size() &&
                index.column() >= 0 &&
                index.column() < 2)
            {
                switch (role)
                {
                case Qt::DisplayRole:
                    out.setValue(QString::fromUtf8(p.views[index.row()].c_str()));
                    break;
                case Qt::BackgroundRole:
                    if (index.row() == p.viewIndex)
                    {
                        out.setValue(
                            QBrush(qApp->palette().color(QPalette::ColorRole::Highlight)));
                    }
                    break;
                case Qt::ForegroundRole:
                    if (index.row() == p.viewIndex)
                    {
                        out.setValue(
                            QBrush(qApp->palette().color(QPalette::ColorRole::HighlightedText)));
                    }
                    break;
                default: break;
                }
            }
            return out;
        }
    }
}
