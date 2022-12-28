// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2022 Darby Johnston
// All rights reserved.

#include <tlPlayApp/FilesAModel.h>

#include <QApplication>
#include <QPalette>

namespace tl
{
    namespace play
    {
        struct FilesAModel::Private
        {
            std::shared_ptr<FilesModelItem> a;
            std::shared_ptr<observer::ValueObserver<std::shared_ptr<FilesModelItem> > > aObserver;
        };

        FilesAModel::FilesAModel(
            const std::shared_ptr<FilesModel>& filesModel,
            qt::TimelineThumbnailProvider* thumbnailProvider,
            const std::shared_ptr<system::Context>& context,
            QObject* parent) :
            FilesTableModel(filesModel, thumbnailProvider, context, parent),
            _p(new Private)
        {
            TLRENDER_P();

            p.aObserver = observer::ValueObserver<std::shared_ptr<FilesModelItem> >::create(
                filesModel->observeA(),
                [this](const std::shared_ptr<FilesModelItem>& value)
                {
                    const int prevIndex = _index(_p->a);
                    _p->a = value;
                    const int index = _index(_p->a);
                    Q_EMIT dataChanged(
                        this->index(index, 0),
                        this->index(index, 1),
                        { Qt::BackgroundRole, Qt::ForegroundRole });
                    Q_EMIT dataChanged(
                        this->index(prevIndex, 0),
                        this->index(prevIndex, 1),
                        { Qt::BackgroundRole, Qt::ForegroundRole });
                });
        }

        FilesAModel::~FilesAModel()
        {}

        QVariant FilesAModel::data(const QModelIndex& index, int role) const
        {
            TLRENDER_P();
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
                    const int aIndex = _index(p.a);
                    if (aIndex == index.row())
                    {
                        out.setValue(QBrush(qApp->palette().color(QPalette::ColorRole::Highlight)));
                    }
                    break;
                }
                case Qt::ForegroundRole:
                {
                    const int aIndex = _index(p.a);
                    if (aIndex == index.row())
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
    }
}
