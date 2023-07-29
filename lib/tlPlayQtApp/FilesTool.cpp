// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2023 Darby Johnston
// All rights reserved.

#include <tlPlayQtApp/FilesTool.h>

#include <tlPlayQtApp/App.h>
#include <tlPlayQtApp/DockTitleBar.h>
#include <tlPlayQtApp/FilesAModel.h>
#include <tlPlayQtApp/FilesView.h>
#include <tlPlayQtApp/SettingsObject.h>

#include <QBoxLayout>
#include <QHeaderView>
#include <QLabel>
#include <QSignalBlocker>
#include <QToolBar>
#include <QTreeView>

namespace tl
{
    namespace play_qt
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
            IToolWidget(app, parent),
            _p(new Private)
        {
            TLRENDER_P();

            p.app = app;
            p.filesAModel = new FilesAModel(
                app->filesModel(),
                app->thumbnailObject(),
                app->getContext(),
                this);

            p.treeView = new QTreeView;
            p.treeView->setAllColumnsShowFocus(true);
            p.treeView->setAlternatingRowColors(true);
            p.treeView->setSelectionMode(QAbstractItemView::NoSelection);
            p.treeView->setItemDelegateForColumn(1, new FilesLayersItemDelegate);
            p.treeView->setEditTriggers(QAbstractItemView::CurrentChanged);
            p.treeView->setIndentation(0);
            p.treeView->setHorizontalScrollMode(QAbstractItemView::ScrollPerPixel);
            p.treeView->setVerticalScrollMode(QAbstractItemView::ScrollPerPixel);
            p.treeView->setModel(p.filesAModel);

            auto toolBar = new QToolBar;
            toolBar->setToolButtonStyle(Qt::ToolButtonIconOnly);
            toolBar->setIconSize(QSize(20, 20));
            toolBar->addAction(actions["Open"]);
            toolBar->addAction(actions["OpenSeparateAudio"]);
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

            auto settingsObject = app->settingsObject();
            settingsObject->setDefaultValue("FilesTool/Header", QByteArray());
            auto ba = settingsObject->value("FilesTool/Header").toByteArray();
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

        FilesDockWidget::FilesDockWidget(
            FilesTool* filesTool,
            QWidget* parent)
        {
            setObjectName("FilesTool");
            setWindowTitle(tr("Files"));
            setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);

            auto dockTitleBar = new DockTitleBar;
            dockTitleBar->setText(tr("Files"));
            dockTitleBar->setIcon(QIcon(":/Icons/Files.svg"));
            auto dockWidget = new QDockWidget;
            setTitleBarWidget(dockTitleBar);

            setWidget(filesTool);

            toggleViewAction()->setIcon(QIcon(":/Icons/Files.svg"));
            toggleViewAction()->setShortcut(QKeySequence(Qt::Key_F1));
            toggleViewAction()->setToolTip(tr("Show files"));
        }
    }
}
