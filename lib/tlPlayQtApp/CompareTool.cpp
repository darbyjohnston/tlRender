// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2023 Darby Johnston
// All rights reserved.

#include <tlPlayQtApp/CompareTool.h>

#include <tlPlayQtApp/App.h>
#include <tlPlayQtApp/DockTitleBar.h>
#include <tlPlayQtApp/FilesBModel.h>
#include <tlPlayQtApp/FilesView.h>
#include <tlPlayQtApp/SettingsObject.h>

#include <tlQtWidget/FloatEditSlider.h>

#include <QBoxLayout>
#include <QFormLayout>
#include <QHeaderView>
#include <QLabel>
#include <QSignalBlocker>
#include <QToolBar>
#include <QTreeView>

namespace tl
{
    namespace play_qt
    {
        struct CompareTool::Private
        {
            App* app = nullptr;
            FilesBModel* filesBModel = nullptr;

            QTreeView* treeView = nullptr;
            qtwidget::FloatEditSlider* wipeXSlider = nullptr;
            qtwidget::FloatEditSlider* wipeYSlider = nullptr;
            qtwidget::FloatEditSlider* wipeRotationSlider = nullptr;
            qtwidget::FloatEditSlider* overlaySlider = nullptr;

            std::shared_ptr<observer::ValueObserver<timeline::CompareOptions> > compareOptionsObserver;
        };

        CompareTool::CompareTool(
            const QMap<QString, QAction*>& actions,
            App* app,
            QWidget* parent) :
            IToolWidget(app, parent),
            _p(new Private)
        {
            TLRENDER_P();

            p.app = app;

            p.filesBModel = new FilesBModel(
                app->filesModel(),
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

            p.wipeXSlider = new qtwidget::FloatEditSlider;

            p.wipeYSlider = new qtwidget::FloatEditSlider;

            p.wipeRotationSlider = new qtwidget::FloatEditSlider;
            p.wipeRotationSlider->setRange(math::FloatRange(0.F, 360.F));

            p.overlaySlider = new qtwidget::FloatEditSlider;

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

            auto settingsObject = app->settingsObject();
            settingsObject->setDefaultValue("CompareTool/Header", QByteArray());
            auto ba = settingsObject->value("CompareTool/Header").toByteArray();
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
                &qtwidget::FloatEditSlider::valueChanged,
                [this, app](double value)
                {
                    auto options = app->filesModel()->getCompareOptions();
                    options.wipeCenter.x = value;
                    app->filesModel()->setCompareOptions(options);
                });

            connect(
                p.wipeYSlider,
                &qtwidget::FloatEditSlider::valueChanged,
                [this, app](double value)
                {
                    auto options = app->filesModel()->getCompareOptions();
                    options.wipeCenter.y = value;
                    app->filesModel()->setCompareOptions(options);
                });

            connect(
                p.wipeRotationSlider,
                &qtwidget::FloatEditSlider::valueChanged,
                [this, app](double value)
                {
                    auto options = app->filesModel()->getCompareOptions();
                    options.wipeRotation = value;
                    app->filesModel()->setCompareOptions(options);
                });

            connect(
                p.overlaySlider,
                &qtwidget::FloatEditSlider::valueChanged,
                [this, app](double value)
                {
                    auto options = app->filesModel()->getCompareOptions();
                    options.overlay = value;
                    app->filesModel()->setCompareOptions(options);
                });

            p.compareOptionsObserver = observer::ValueObserver<timeline::CompareOptions>::create(
                app->filesModel()->observeCompareOptions(),
                [this](const timeline::CompareOptions& value)
                {
                    _widgetUpdate();
                });
        }

        CompareTool::~CompareTool()
        {
            TLRENDER_P();
            p.app->settingsObject()->setValue(
                "CompareTool/Header",
                p.treeView->header()->saveState());
        }

        void CompareTool::_activatedCallback(const QModelIndex& index)
        {
            TLRENDER_P();
            p.app->filesModel()->toggleB(index.row());
        }

        void CompareTool::_widgetUpdate()
        {
            TLRENDER_P();
            const auto options = p.app->filesModel()->getCompareOptions();
            {
                QSignalBlocker signalBlocker(p.wipeXSlider);
                p.wipeXSlider->setValue(options.wipeCenter.x);
            }
            {
                QSignalBlocker signalBlocker(p.wipeYSlider);
                p.wipeYSlider->setValue(options.wipeCenter.y);
            }
            {
                QSignalBlocker signalBlocker(p.wipeYSlider);
                p.wipeRotationSlider->setValue(options.wipeRotation);
            }
            {
                QSignalBlocker signalBlocker(p.overlaySlider);
                p.overlaySlider->setValue(options.overlay);
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
            dockTitleBar->setText(tr("Compare"));
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
