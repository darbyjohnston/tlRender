// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2022 Darby Johnston
// All rights reserved.

#include <tlPlay/FilesTool.h>

#include <tlPlay/FilesModel.h>
#include <tlPlay/FilesView.h>

#include <tlQt/Util.h>

#include <QBoxLayout>
#include <QHeaderView>
#include <QLabel>
#include <QSignalBlocker>
#include <QSettings>
#include <QTreeView>

namespace tl
{
    namespace play
    {
        struct FilesTool::Private
        {
            std::shared_ptr<FilesModel> filesModel;
            FilesAModel* filesAModel = nullptr;
            QTreeView* treeView = nullptr;
        };

        FilesTool::FilesTool(
            const std::shared_ptr<FilesModel>& filesModel,
            const std::shared_ptr<core::Context>& context,
            QWidget* parent) :
            ToolWidget(parent),
            _p(new Private)
        {
            TLRENDER_P();

            p.filesModel = filesModel;

            p.filesAModel = new FilesAModel(filesModel, context, this);

            p.treeView = new QTreeView;
            p.treeView->setAllColumnsShowFocus(true);
            p.treeView->setAlternatingRowColors(true);
            p.treeView->setSelectionMode(QAbstractItemView::NoSelection);
            p.treeView->setItemDelegateForColumn(1, new FilesLayersItemDelegate);
            p.treeView->setEditTriggers(QAbstractItemView::CurrentChanged);
            p.treeView->setIndentation(0);
            //! \bug Setting the model causes this output to be printed on exit:
            //! QBasicTimer::start: QBasicTimer can only be used with threads started with QThread
            p.treeView->setModel(p.filesAModel);

            auto vLayout = new QVBoxLayout;
            vLayout->setContentsMargins(0, 0, 0, 0);
            vLayout->addWidget(p.treeView);
            auto viewWidget = new QWidget;
            viewWidget->setLayout(vLayout);
            addWidget(viewWidget, 1);

            QSettings settings;
            auto ba = settings.value(qt::versionedSettingsKey("FilesTool/Header")).toByteArray();
            if (!ba.isEmpty())
            {
                p.treeView->header()->restoreState(ba);
            }

            connect(
                p.treeView,
                SIGNAL(activated(const QModelIndex&)),
                SLOT(_activatedCallback(const QModelIndex&)));
        }

        FilesTool::~FilesTool()
        {
            TLRENDER_P();
            QSettings settings;
            settings.setValue(qt::versionedSettingsKey("FilesTool/Header"), p.treeView->header()->saveState());
        }

        void FilesTool::_activatedCallback(const QModelIndex& index)
        {
            TLRENDER_P();
            p.filesModel->setA(index.row());
        }
    }
}
