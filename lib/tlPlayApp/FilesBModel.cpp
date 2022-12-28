// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2022 Darby Johnston
// All rights reserved.

#include <tlPlayApp/FilesBModel.h>

#include <QApplication>
#include <QPalette>

namespace tl
{
    namespace play
    {
        struct FilesBModel::Private
        {
            std::vector<std::shared_ptr<FilesModelItem> > b;
            std::shared_ptr<observer::ListObserver<std::shared_ptr<FilesModelItem> > > bObserver;
        };

        FilesBModel::FilesBModel(
            const std::shared_ptr<FilesModel>& filesModel,
            qt::TimelineThumbnailProvider* thumbnailProvider,
            const std::shared_ptr<system::Context>& context,
            QObject* parent) :
            FilesTableModel(filesModel, thumbnailProvider, context, parent),
            _p(new Private)
        {
            TLRENDER_P();

            p.bObserver = observer::ListObserver<std::shared_ptr<FilesModelItem> >::create(
                filesModel->observeB(),
                [this](const std::vector<std::shared_ptr<FilesModelItem> >& value)
                {
                    const auto prevIndexes = _bIndexes();
                    _p->b = value;
                    for (const auto& i : _bIndexes())
                    {
                        Q_EMIT dataChanged(
                            this->index(i, 0),
                            this->index(i, 1),
                            { Qt::BackgroundRole, Qt::ForegroundRole });
                    }
                    for (const auto& i : prevIndexes)
                    {
                        Q_EMIT dataChanged(
                            this->index(i, 0),
                            this->index(i, 1),
                            { Qt::BackgroundRole, Qt::ForegroundRole });
                    }
                });
        }

        FilesBModel::~FilesBModel()
        {}

        QVariant FilesBModel::data(const QModelIndex& index, int role) const
        {
            QVariant out = FilesTableModel::data(index, role);
            if (index.isValid() &&
                index.row() >= 0 &&
                index.row() < _files.size() &&
                index.column() >= 0 &&
                index.column() < 2)
            {
                const auto& item = _files[index.row()];
                switch (role)
                {
                case Qt::BackgroundRole:
                {
                    const auto bIndexes = _bIndexes();
                    const auto i = std::find(bIndexes.begin(), bIndexes.end(), index.row());
                    if (i != bIndexes.end())
                    {
                        out.setValue(QBrush(qApp->palette().color(QPalette::ColorRole::Highlight)));
                    }
                    break;
                }
                case Qt::ForegroundRole:
                {
                    const auto bIndexes = _bIndexes();
                    const auto i = std::find(bIndexes.begin(), bIndexes.end(), index.row());
                    if (i != bIndexes.end())
                    {
                        out.setValue(QBrush(qApp->palette().color(QPalette::ColorRole::HighlightedText)));
                    }
                    break;
                }
                default: break;
                }
            }
            return out;
        }

        std::vector<int> FilesBModel::_bIndexes() const
        {
            TLRENDER_P();
            std::vector<int> out;
            for (const auto& b : p.b)
            {
                out.push_back(_index(b));
            }
            return out;
        }
    }
}
