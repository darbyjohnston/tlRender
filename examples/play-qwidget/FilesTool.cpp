// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2022 Darby Johnston
// All rights reserved.

#include "App.h"
#include "FilesTool.h"
#include "FilesView.h"

#include <QBoxLayout>
#include <QSignalBlocker>

namespace tlr
{
    FilesTool::FilesTool(
        FilesModel* filesModel,
        QWidget* parent) :
        QWidget(parent),
        _filesModel(filesModel)
    {
        _treeView = new QTreeView;
        _treeView->setAllColumnsShowFocus(true);
        _treeView->setAlternatingRowColors(true);
        _treeView->setSelectionMode(QAbstractItemView::NoSelection);
        _treeView->setItemDelegateForColumn(3, new FilesLayersItemDelegate);
        _treeView->setEditTriggers(QAbstractItemView::CurrentChanged);
        _treeView->setModel(filesModel);
        _treeView->resizeColumnToContents(0);
        _treeView->resizeColumnToContents(1);

        auto openButton = new QToolButton;
        openButton->setIcon(QIcon(":/Icons/FileOpen.svg"));
        openButton->setToolTip(tr("Open a new file"));
        auto openWithAudioButton = new QToolButton;
        openWithAudioButton->setIcon(QIcon(":/Icons/FileOpenWithAudio.svg"));
        openWithAudioButton->setToolTip(tr("Open a new file with audio"));

        _closeButton = new QToolButton;
        _closeButton->setIcon(QIcon(":/Icons/FileClose.svg"));
        _closeButton->setToolTip(tr("Close the current file"));
        _closeAllButton = new QToolButton;
        _closeAllButton->setIcon(QIcon(":/Icons/FileCloseAll.svg"));
        _closeAllButton->setToolTip(tr("Close all files"));

        _nextButton = new QToolButton;
        _nextButton->setIcon(QIcon(":/Icons/FileNext.svg"));
        _nextButton->setToolTip(tr("Go to the next file"));
        _prevButton = new QToolButton;
        _prevButton->setIcon(QIcon(":/Icons/FilePrev.svg"));
        _prevButton->setToolTip(tr("Go to the previous file"));

        auto layout = new QVBoxLayout;
        layout->addWidget(_treeView);
        auto hLayout = new QHBoxLayout;
        auto hLayout2 = new QHBoxLayout;
        hLayout2->setSpacing(1);
        hLayout2->addWidget(openButton);
        hLayout2->addWidget(openWithAudioButton);
        hLayout2->addWidget(_closeButton);
        hLayout2->addWidget(_closeAllButton);
        hLayout->addLayout(hLayout2);
        hLayout->addStretch();
        hLayout2 = new QHBoxLayout;
        hLayout2->setSpacing(1);
        hLayout2->addWidget(_prevButton);
        hLayout2->addWidget(_nextButton);
        hLayout->addLayout(hLayout2);
        layout->addLayout(hLayout);
        setLayout(layout);

        _countUpdate();

        connect(
            _treeView,
            SIGNAL(activated(const QModelIndex&)),
            SLOT(_activatedCallback(const QModelIndex&)));

        connect(
            openButton,
            SIGNAL(clicked()),
            qApp,
            SLOT(open()));
        connect(
            openWithAudioButton,
            SIGNAL(clicked()),
            qApp,
            SLOT(openWithAudio()));

        connect(
            _closeButton,
            SIGNAL(clicked()),
            filesModel,
            SLOT(close()));
        connect(
            _closeAllButton,
            SIGNAL(clicked()),
            filesModel,
            SLOT(closeAll()));

        connect(
            _nextButton,
            SIGNAL(clicked()),
            filesModel,
            SLOT(next()));
        connect(
            _prevButton,
            SIGNAL(clicked()),
            filesModel,
            SLOT(prev()));

        connect(
            filesModel,
            SIGNAL(countChanged(int)),
            SLOT(_countCallback()));
    }

    void FilesTool::_activatedCallback(const QModelIndex& index)
    {
        _filesModel->setA(index.row());
    }

    void FilesTool::_countCallback()
    {
        _countUpdate();
    }

    void FilesTool::_countUpdate()
    {
        const int count = _filesModel->rowCount();
        _closeButton->setEnabled(count > 0);
        _closeAllButton->setEnabled(count > 0);
        _nextButton->setEnabled(count > 1);
        _prevButton->setEnabled(count > 1);
    }
}
