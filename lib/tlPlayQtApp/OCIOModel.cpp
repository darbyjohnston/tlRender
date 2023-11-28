// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2023 Darby Johnston
// All rights reserved.

#include <tlPlayQtApp/OCIOModel.h>

#include <QApplication>
#include <QPalette>

namespace tl
{
    namespace play_qt
    {
        struct OCIOInputListModel::Private
        {
            std::vector<std::string> inputs;
            size_t inputIndex = 0;
            std::shared_ptr<observer::ValueObserver<play::OCIOModelData> > dataObserver;
        };

        OCIOInputListModel::OCIOInputListModel(
            const std::shared_ptr<play::OCIOModel>& ocioModel,
            QObject* parent) :
            QAbstractListModel(parent),
            _p(new Private)
        {
            TLRENDER_P();

            p.dataObserver = observer::ValueObserver<play::OCIOModelData>::create(
                ocioModel->observeData(),
                [this](const play::OCIOModelData& value)
                {
                    beginResetModel();
                    _p->inputs = value.inputs;
                    _p->inputIndex = value.inputIndex;
                    endResetModel();
                });
        }

        OCIOInputListModel::~OCIOInputListModel()
        {}

        int OCIOInputListModel::rowCount(const QModelIndex& parent) const
        {
            return _p->inputs.size();
        }

        QVariant OCIOInputListModel::data(const QModelIndex& index, int role) const
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

        struct OCIODisplayListModel::Private
        {
            std::vector<std::string> displays;
            size_t displayIndex = 0;
            std::shared_ptr<observer::ValueObserver<play::OCIOModelData> > dataObserver;
        };

        OCIODisplayListModel::OCIODisplayListModel(
            const std::shared_ptr<play::OCIOModel>& ocioModel,
            QObject* parent) :
            QAbstractListModel(parent),
            _p(new Private)
        {
            TLRENDER_P();

            p.dataObserver = observer::ValueObserver<play::OCIOModelData>::create(
                ocioModel->observeData(),
                [this](const play::OCIOModelData& value)
                {
                    beginResetModel();
                    _p->displays = value.displays;
                    _p->displayIndex = value.displayIndex;
                    endResetModel();
                });
        }

        OCIODisplayListModel::~OCIODisplayListModel()
        {}

        int OCIODisplayListModel::rowCount(const QModelIndex& parent) const
        {
            return _p->displays.size();
        }

        QVariant OCIODisplayListModel::data(const QModelIndex& index, int role) const
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

        struct OCIOViewListModel::Private
        {
            std::vector<std::string> views;
            size_t viewIndex = 0;
            std::shared_ptr<observer::ValueObserver<play::OCIOModelData> > dataObserver;
        };

        OCIOViewListModel::OCIOViewListModel(
            const std::shared_ptr<play::OCIOModel>& ocioModel,
            QObject* parent) :
            QAbstractListModel(parent),
            _p(new Private)
        {
            TLRENDER_P();

            p.dataObserver = observer::ValueObserver<play::OCIOModelData>::create(
                ocioModel->observeData(),
                [this](const play::OCIOModelData& value)
                {
                    beginResetModel();
                    _p->views = value.views;
                    _p->viewIndex = value.viewIndex;
                    endResetModel();
                });
        }

        OCIOViewListModel::~OCIOViewListModel()
        {}

        int OCIOViewListModel::rowCount(const QModelIndex& parent) const
        {
            return _p->views.size();
        }

        QVariant OCIOViewListModel::data(const QModelIndex& index, int role) const
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
