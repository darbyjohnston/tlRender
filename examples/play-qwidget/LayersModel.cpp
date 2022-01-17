// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2022 Darby Johnston
// All rights reserved.

#include "LayersModel.h"

#include <tlrCore/Math.h>
#include <tlrCore/StringFormat.h>

#include <QApplication>
#include <QPalette>

namespace tlr
{
    LayersModel::LayersModel(QObject* parent) :
        QAbstractListModel(parent)
    {}

    void LayersModel::set(const std::vector<imaging::Info>& items, int current)
    {
        beginResetModel();
        _items = items;
        endResetModel();
        _current = math::clamp(current, 0, static_cast<int>(_items.size()) - 1);
        Q_EMIT currentChanged(_current);
        Q_EMIT countChanged(_items.size());
        Q_EMIT dataChanged(
            this->index(_current, 0),
            this->index(_current, 0),
            { Qt::BackgroundRole, Qt::ForegroundRole });
    }

    int LayersModel::current() const
    {
        return _current;
    }

    void LayersModel::setCurrent(int index)
    {
        if (index >= 0 &&
            index < _items.size() &&
            index != _current)
        {
            _current = index;
            Q_EMIT currentChanged(_current);
            Q_EMIT dataChanged(
                this->index(_current, 0),
                this->index(_current, 0),
                { Qt::BackgroundRole, Qt::ForegroundRole });
        }
    }

    int LayersModel::rowCount(const QModelIndex&) const
    {
        return _items.size();
    }

    QVariant LayersModel::data(const QModelIndex& index, int role) const
    {
        QVariant out;
        if (index.isValid() &&
            index.row() >= 0 &&
            index.row() < _items.size())
        {
            switch (role)
            {
            case Qt::DisplayRole:
            {
                const auto& item = _items[index.row()];
                std::string s = string::Format(
                    "{0}\n"
                    "    {1}").
                    arg(item.name).
                    arg(item);
                out.setValue(QString::fromUtf8(s.c_str()));
                break;
            }
            case Qt::BackgroundRole:
                out = qApp->palette().color(index.row() == _current ?
                    QPalette::Highlight :
                    QPalette::Base);
                break;
            case Qt::ForegroundRole:
                out = qApp->palette().color(index.row() == _current ?
                    QPalette::HighlightedText :
                    QPalette::Foreground);
                break;
            default: break;
            }
        }
        return out;
    }

    void LayersModel::first()
    {
        if (!_items.empty() && _current != 0)
        {
            _current = 0;
            Q_EMIT currentChanged(_current);
            if (_current != -1)
            {
                Q_EMIT dataChanged(
                    index(_current, 0),
                    index(_current, 0),
                    { Qt::BackgroundRole, Qt::ForegroundRole });
            }
        }
    }

    void LayersModel::last()
    {
        if (!_items.empty() && _current != _items.size() - 1)
        {
            _current = _items.size() - 1;
            Q_EMIT currentChanged(_current);
            if (_current != -1)
            {
                Q_EMIT dataChanged(
                    index(_current, 0),
                    index(_current, 0),
                    { Qt::BackgroundRole, Qt::ForegroundRole });
            }
        }
    }

    void LayersModel::next()
    {
        if (_items.size() > 1)
        {
            ++_current;
            if (_current >= _items.size())
            {
                _current = 0;
            }
            Q_EMIT currentChanged(_current);
            if (_current != -1)
            {
                Q_EMIT dataChanged(
                    index(_current, 0),
                    index(_current, 0),
                    { Qt::BackgroundRole, Qt::ForegroundRole });
            }
        }
    }

    void LayersModel::prev()
    {
        if (_items.size() > 1)
        {
            --_current;
            if (_current < 0)
            {
                _current = _items.size() - 1;
            }
            Q_EMIT currentChanged(_current);
            if (_current != -1)
            {
                Q_EMIT dataChanged(
                    index(_current, 0),
                    index(_current, 0),
                    { Qt::BackgroundRole, Qt::ForegroundRole });
            }
        }
    }
}
