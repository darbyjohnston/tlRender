// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2022 Darby Johnston
// All rights reserved.

#include <tlAppPlay/FilesTool.h>

#include <tlAppPlay/App.h>
#include <tlAppPlay/FilesModel.h>
#include <tlAppPlay/FilesView.h>
#include <tlAppPlay/SettingsObject.h>

#include <tlQt/Util.h>

#include <QBoxLayout>
#include <QHeaderView>
#include <QLabel>
#include <QSignalBlocker>
#include <QToolBar>
#include <QTreeView>

namespace tl
{
    namespace play
    {
        struct FilesTool::Private
        {
            App* app = nullptr;
            FilesAModel* filesAModel = nullptr;
            QTreeView* treeView = nullptr;
        };

        FilesTool::FilesTool(
            const QMap<QString, QAction*>& actions,
            App* app,
            QWidget* parent) :
            ToolWidget(parent),
            _p(new Private)
        {
            TLRENDER_P();

            p.app = app;
            p.filesAModel = new FilesAModel(app->filesModel(), app->getContext(), this);

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

            auto toolBar = new QToolBar;
            toolBar->setToolButtonStyle(Qt::ToolButtonIconOnly);
            toolBar->setIconSize(QSize(20, 20));
            toolBar->addAction(actions["Open"]);
            toolBar->addAction(actions["OpenWithAudio"]);
            toolBar->addAction(actions["Close"]);
            toolBar->addAction(actions["CloseAll"]);
            toolBar->addSeparator();
            toolBar->addAction(actions["Prev"]);
            toolBar->addAction(actions["Next"]);

            auto layout = new QVBoxLayout;
            layout->setContentsMargins(0, 0, 0, 0);
            layout->setSpacing(0);
            layout->addWidget(p.treeView);
            layout->addWidget(toolBar);
            auto widget = new QWidget;
            widget->setLayout(layout);
            addWidget(widget, 1);

            app->settingsObject()->setDefaultValue("FilesTool/Header", QByteArray());
            auto ba = app->settingsObject()->value("FilesTool/Header").toByteArray();
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
            p.app->settingsObject()->setValue(
                "FilesTool/Header",
                p.treeView->header()->saveState());
        }

        void FilesTool::_activatedCallback(const QModelIndex& index)
        {
            TLRENDER_P();
            p.app->filesModel()->setA(index.row());
        }
    }
}
