// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2022 Darby Johnston
// All rights reserved.

#include <tlPlayApp/CompareTool.h>

#include <tlPlayApp/App.h>
#include <tlPlayApp/DockTitleBar.h>
#include <tlPlayApp/FilesBModel.h>
#include <tlPlayApp/FilesView.h>
#include <tlPlayApp/SettingsObject.h>

#include <tlQtWidget/FloatSlider.h>

#include <tlQt/Util.h>

#include <QBoxLayout>
#include <QFormLayout>
#include <QHeaderView>
#include <QLabel>
#include <QSignalBlocker>
#include <QToolBar>
#include <QTreeView>

namespace tl
{
    namespace play
    {
        struct CompareTool::Private
        {
            App* app = nullptr;
            FilesBModel* filesBModel = nullptr;
            timeline::CompareOptions compareOptions;
            QTreeView* treeView = nullptr;
            qtwidget::FloatSlider* wipeXSlider = nullptr;
            qtwidget::FloatSlider* wipeYSlider = nullptr;
            qtwidget::FloatSlider* wipeRotationSlider = nullptr;
            qtwidget::FloatSlider* overlaySlider = nullptr;
        };

        CompareTool::CompareTool(
            const QMap<QString, QAction*>& actions,
            App* app,
            QWidget* parent) :
            ToolWidget(parent),
            _p(new Private)
        {
            TLRENDER_P();

            p.app = app;

            p.filesBModel = new FilesBModel(
                app->filesModel(),
                app->thumbnailProvider(),
                app->getContext(),
                this);

            p.treeView = new QTreeView;
            p.treeView->setAllColumnsShowFocus(true);
            p.treeView->setAlternatingRowColors(true);
            p.treeView->setSelectionMode(QAbstractItemView::NoSelection);
            p.treeView->setItemDelegateForColumn(1, new FilesLayersItemDelegate);
            p.treeView->setEditTriggers(QAbstractItemView::CurrentChanged);
            p.treeView->setHorizontalScrollMode(QAbstractItemView::ScrollPerPixel);
            p.treeView->setVerticalScrollMode(QAbstractItemView::ScrollPerPixel);
            p.treeView->setIndentation(0);
            p.treeView->setModel(p.filesBModel);

            auto toolBar = new QToolBar;
            toolBar->setToolButtonStyle(Qt::ToolButtonIconOnly);
            toolBar->setIconSize(QSize(20, 20));
            toolBar->addAction(actions["A"]);
            toolBar->addAction(actions["B"]);
            toolBar->addAction(actions["Wipe"]);
            toolBar->addAction(actions["Overlay"]);
            toolBar->addAction(actions["Difference"]);
            toolBar->addAction(actions["Horizontal"]);
            toolBar->addAction(actions["Vertical"]);
            toolBar->addAction(actions["Tile"]);
            toolBar->addSeparator();
            toolBar->addAction(actions["Prev"]);
            toolBar->addAction(actions["Next"]);

            p.wipeXSlider = new qtwidget::FloatSlider;

            p.wipeYSlider = new qtwidget::FloatSlider;

            p.wipeRotationSlider = new qtwidget::FloatSlider;
            p.wipeRotationSlider->setRange(math::FloatRange(0.F, 360.F));

            p.overlaySlider = new qtwidget::FloatSlider;

            auto layout = new QVBoxLayout;
            layout->setContentsMargins(0, 0, 0, 0);
            layout->setSpacing(0);
            layout->addWidget(p.treeView);
            layout->addWidget(toolBar);
            auto widget = new QWidget;
            widget->setLayout(layout);
            addWidget(widget, 1);

            auto formLayout = new QFormLayout;
            formLayout->addRow(tr("X:"), p.wipeXSlider);
            formLayout->addRow(tr("Y:"), p.wipeYSlider);
            formLayout->addRow(tr("Rotation:"), p.wipeRotationSlider);
            widget = new QWidget;
            widget->setLayout(formLayout);
            addBellows(tr("Wipe"), widget);

            layout = new QVBoxLayout;
            layout->addWidget(p.overlaySlider);
            widget = new QWidget;
            widget->setLayout(layout);
            addBellows(tr("Overlay"), widget);

            _widgetUpdate();

            app->settingsObject()->setDefaultValue("CompareTool/Header", QByteArray());
            auto ba = app->settingsObject()->value("CompareTool/Header").toByteArray();
            if (!ba.isEmpty())
            {
                p.treeView->header()->restoreState(ba);
            }

            connect(
                p.treeView,
                SIGNAL(activated(const QModelIndex&)),
                SLOT(_activatedCallback(const QModelIndex&)));

            connect(
                p.wipeXSlider,
                &qtwidget::FloatSlider::valueChanged,
                [this](double value)
                {
                    timeline::CompareOptions compareOptions = _p->compareOptions;
                    compareOptions.wipeCenter.x = value;
                    Q_EMIT compareOptionsChanged(compareOptions);
                });

            connect(
                p.wipeYSlider,
                &qtwidget::FloatSlider::valueChanged,
                [this](double value)
                {
                    timeline::CompareOptions compareOptions = _p->compareOptions;
                    compareOptions.wipeCenter.y = value;
                    Q_EMIT compareOptionsChanged(compareOptions);
                });

            connect(
                p.wipeRotationSlider,
                &qtwidget::FloatSlider::valueChanged,
                [this](double value)
                {
                    timeline::CompareOptions compareOptions = _p->compareOptions;
                    compareOptions.wipeRotation = value;
                    Q_EMIT compareOptionsChanged(compareOptions);
                });

            connect(
                p.overlaySlider,
                &qtwidget::FloatSlider::valueChanged,
                [this](double value)
                {
                    timeline::CompareOptions compareOptions = _p->compareOptions;
                    compareOptions.overlay = value;
                    Q_EMIT compareOptionsChanged(compareOptions);
                });
        }

        CompareTool::~CompareTool()
        {
            TLRENDER_P();
            p.app->settingsObject()->setValue(
                "CompareTool/Header",
                p.treeView->header()->saveState());
        }

        void CompareTool::setCompareOptions(const timeline::CompareOptions& value)
        {
            TLRENDER_P();
            if (value == p.compareOptions)
                return;
            p.compareOptions = value;
            _widgetUpdate();
        }

        void CompareTool::_activatedCallback(const QModelIndex& index)
        {
            TLRENDER_P();
            p.app->filesModel()->toggleB(index.row());
        }

        void CompareTool::_widgetUpdate()
        {
            TLRENDER_P();
            {
                QSignalBlocker signalBlocker(p.wipeXSlider);
                p.wipeXSlider->setValue(p.compareOptions.wipeCenter.x);
            }
            {
                QSignalBlocker signalBlocker(p.wipeYSlider);
                p.wipeYSlider->setValue(p.compareOptions.wipeCenter.y);
            }
            {
                QSignalBlocker signalBlocker(p.wipeYSlider);
                p.wipeRotationSlider->setValue(p.compareOptions.wipeRotation);
            }
            {
                QSignalBlocker signalBlocker(p.overlaySlider);
                p.overlaySlider->setValue(p.compareOptions.overlay);
            }
        }

        CompareDockWidget::CompareDockWidget(
            CompareTool* compareTool,
            QWidget* parent)
        {
            setObjectName("CompareTool");
            setWindowTitle(tr("Compare"));
            setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);

            auto dockTitleBar = new DockTitleBar;
            dockTitleBar->setText(tr("COMPARE"));
            dockTitleBar->setIcon(QIcon(":/Icons/Compare.svg"));
            auto dockWidget = new QDockWidget;
            setTitleBarWidget(dockTitleBar);

            setWidget(compareTool);

            toggleViewAction()->setIcon(QIcon(":/Icons/Compare.svg"));
            toggleViewAction()->setShortcut(QKeySequence(Qt::Key_F2));
            toggleViewAction()->setToolTip(tr("Show compare controls"));
        }
    }
}
