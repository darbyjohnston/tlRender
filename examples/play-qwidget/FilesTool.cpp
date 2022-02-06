// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2022 Darby Johnston
// All rights reserved.

#include "FilesTool.h"
#include "FilesView.h"

#include <QBoxLayout>
#include <QHeaderView>
#include <QLabel>
#include <QSignalBlocker>
#include <QSettings>

namespace tlr
{
    FilesTool::FilesTool(
        const std::shared_ptr<FilesModel>& filesModel,
        const std::shared_ptr<core::Context>& context,
        QWidget* parent) :
        ToolWidget(parent),
        _filesModel(filesModel)
    {
        _filesAModel = new FilesAModel(filesModel, context, this);

        _treeView = new QTreeView;
        _treeView->setAllColumnsShowFocus(true);
        _treeView->setAlternatingRowColors(true);
        _treeView->setSelectionMode(QAbstractItemView::NoSelection);
        _treeView->setItemDelegateForColumn(1, new FilesLayersItemDelegate);
        _treeView->setEditTriggers(QAbstractItemView::CurrentChanged);
        _treeView->setIndentation(0);
        //! \bug Setting the model causes this output to be printed on exit:
        //! QBasicTimer::start: QBasicTimer can only be used with threads started with QThread
        _treeView->setModel(_filesAModel);

        auto vLayout = new QVBoxLayout;
        vLayout->setContentsMargins(0, 0, 0, 0);
        vLayout->addWidget(_treeView);
        auto viewWidget = new QWidget;
        viewWidget->setLayout(vLayout);
        addWidget(viewWidget, 1);

        QSettings settings;
        auto ba = settings.value(qt::versionedSettingsKey("FilesTool/Header")).toByteArray();
        if (!ba.isEmpty())
        {
            _treeView->header()->restoreState(ba);
        }

        connect(
            _treeView,
            SIGNAL(activated(const QModelIndex&)),
            SLOT(_activatedCallback(const QModelIndex&)));
    }

    FilesTool::~FilesTool()
    {
        QSettings settings;
        settings.setValue(qt::versionedSettingsKey("FilesTool/Header"), _treeView->header()->saveState());
    }

    void FilesTool::_activatedCallback(const QModelIndex& index)
    {
        _filesModel->setA(index.row());
    }
}
