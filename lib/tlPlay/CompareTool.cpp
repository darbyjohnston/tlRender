// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2022 Darby Johnston
// All rights reserved.

#include <tlPlay/App.h>
#include <tlPlay/CompareTool.h>
#include <tlPlay/FilesModel.h>
#include <tlPlay/FilesView.h>

#include <tlQWidget/FloatSlider.h>

#include <tlQt/Util.h>

#include <QBoxLayout>
#include <QFormLayout>
#include <QHeaderView>
#include <QLabel>
#include <QSignalBlocker>
#include <QSettings>
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
            render::CompareOptions compareOptions;
            QTreeView* treeView = nullptr;
            qwidget::FloatSlider* wipeXSlider = nullptr;
            qwidget::FloatSlider* wipeYSlider = nullptr;
            qwidget::FloatSlider* wipeRotationSlider = nullptr;
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

            p.filesBModel = new FilesBModel(app->filesModel(), app->getContext(), this);

            p.treeView = new QTreeView;
            p.treeView->setAllColumnsShowFocus(true);
            p.treeView->setAlternatingRowColors(true);
            p.treeView->setSelectionMode(QAbstractItemView::NoSelection);
            p.treeView->setItemDelegateForColumn(1, new FilesLayersItemDelegate);
            p.treeView->setEditTriggers(QAbstractItemView::CurrentChanged);
            p.treeView->setIndentation(0);
            //! \bug Setting the model causes this output to be printed on exit:
            //! QBasicTimer::start: QBasicTimer can only be used with threads started with QThread
            p.treeView->setModel(p.filesBModel);

            auto toolBar = new QToolBar;
            toolBar->setToolButtonStyle(Qt::ToolButtonIconOnly);
            toolBar->setIconSize(QSize(20, 20));
            toolBar->addAction(actions["A"]);
            toolBar->addAction(actions["B"]);
            toolBar->addAction(actions["Wipe"]);
            toolBar->addAction(actions["Tile"]);
            toolBar->addSeparator();
            toolBar->addAction(actions["Prev"]);
            toolBar->addAction(actions["Next"]);

            p.wipeXSlider = new qwidget::FloatSlider;

            p.wipeYSlider = new qwidget::FloatSlider;

            p.wipeRotationSlider = new qwidget::FloatSlider;
            p.wipeRotationSlider->setRange(math::FloatRange(0.F, 360.F));

            auto layout = new QVBoxLayout;
            layout->setContentsMargins(0, 0, 0, 0);
            layout->setSpacing(0);
            layout->addWidget(p.treeView);
            layout->addWidget(toolBar);
            auto vLayout = new QVBoxLayout;
            vLayout->setContentsMargins(10, 10, 10, 10);
            vLayout->setSpacing(10);
            vLayout->addWidget(new QLabel(tr("Wipe")));
            auto formLayout = new QFormLayout;
            formLayout->addRow(tr("X:"), p.wipeXSlider);
            formLayout->addRow(tr("Y:"), p.wipeYSlider);
            formLayout->addRow(tr("Rotation:"), p.wipeRotationSlider);
            vLayout->addLayout(formLayout);
            layout->addLayout(vLayout);
            auto widget = new QWidget;
            widget->setLayout(layout);
            addWidget(widget, 1);

            _widgetUpdate();

            QSettings settings;
            auto ba = settings.value(qt::versionedSettingsKey("CompareTool/Header")).toByteArray();
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
                &qwidget::FloatSlider::valueChanged,
                [this](double value)
                {
                    _p->compareOptions.wipeCenter.x = value;
                    Q_EMIT compareOptionsChanged(_p->compareOptions);
                });

            connect(
                p.wipeYSlider,
                &qwidget::FloatSlider::valueChanged,
                [this](double value)
                {
                    _p->compareOptions.wipeCenter.y = value;
                    Q_EMIT compareOptionsChanged(_p->compareOptions);
                });

            connect(
                p.wipeRotationSlider,
                &qwidget::FloatSlider::valueChanged,
                [this](double value)
                {
                    _p->compareOptions.wipeRotation = value;
                    Q_EMIT compareOptionsChanged(_p->compareOptions);
                });
        }

        CompareTool::~CompareTool()
        {
            TLRENDER_P();
            QSettings settings;
            settings.setValue(qt::versionedSettingsKey("CompareTool/Header"), p.treeView->header()->saveState());
        }

        void CompareTool::setCompareOptions(const render::CompareOptions& value)
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
                p.wipeRotationSlider->setValue(p.compareOptions.wipeRotation / 360.F);
            }
        }
    }
}
