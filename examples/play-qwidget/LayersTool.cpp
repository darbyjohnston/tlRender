// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2022 Darby Johnston
// All rights reserved.

#include "LayersTool.h"

#include <QBoxLayout>
#include <QSignalBlocker>

namespace tlr
{
    LayersTool::LayersTool(
        LayersModel* layersModel,
        QWidget* parent) :
        QWidget(parent),
        _layersModel(layersModel)
    {
        _listView = new QListView;
        _listView->setSelectionMode(QAbstractItemView::NoSelection);
        _listView->setModel(layersModel);

        _nextButton = new QToolButton;
        _nextButton->setIcon(QIcon(":/Icons/LayerNext.svg"));
        _nextButton->setToolTip(tr("Go to the next layer"));
        _prevButton = new QToolButton;
        _prevButton->setIcon(QIcon(":/Icons/LayerPrev.svg"));
        _prevButton->setToolTip(tr("Go to the previous layer"));

        auto layout = new QVBoxLayout;
        layout->addWidget(_listView);
        auto hLayout = new QHBoxLayout;
        hLayout->addStretch();
        auto hLayout2 = new QHBoxLayout;
        hLayout2->setSpacing(1);
        hLayout2->addWidget(_prevButton);
        hLayout2->addWidget(_nextButton);
        hLayout->addLayout(hLayout2);
        layout->addLayout(hLayout);
        setLayout(layout);

        _countUpdate();

        connect(
            _listView,
            SIGNAL(activated(const QModelIndex&)),
            SLOT(_activatedCallback(const QModelIndex&)));

        connect(
            _nextButton,
            SIGNAL(clicked()),
            layersModel,
            SLOT(next()));
        connect(
            _prevButton,
            SIGNAL(clicked()),
            layersModel,
            SLOT(prev()));

        connect(
            layersModel,
            SIGNAL(countChanged(int)),
            SLOT(_countCallback()));
    }

    void LayersTool::_activatedCallback(const QModelIndex& index)
    {
        _layersModel->setCurrent(index.row());
    }

    void LayersTool::_countCallback()
    {
        _countUpdate();
    }

    void LayersTool::_countUpdate()
    {
        const int count = _layersModel->rowCount();
        _nextButton->setEnabled(count > 1);
        _prevButton->setEnabled(count > 1);
    }
}
