// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2022 Darby Johnston
// All rights reserved.

#include "App.h"
#include "FilesTool.h"

#include <QBoxLayout>
#include <QSettings>
#include <QSignalBlocker>

namespace tlr
{
    FilesTool::FilesTool(
        FilesModel* model,
        QWidget* parent) :
        QWidget(parent),
        _model(model)
    {
        _listView = new QListView;
        _listView->setSelectionMode(QAbstractItemView::NoSelection);
        _listView->setModel(model);

        auto layout = new QVBoxLayout;
        layout->addWidget(_listView);
        setLayout(layout);

        connect(
            _listView,
            SIGNAL(activated(const QModelIndex&)),
            SLOT(_activatedCallback(const QModelIndex&)));
    }

    void FilesTool::_activatedCallback(const QModelIndex& index)
    {
        _model->setCurrent(index);
    }
}
