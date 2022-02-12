// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2022 Darby Johnston
// All rights reserved.

#include <tlPlay/InfoTool.h>

#include <tlPlay/InfoModel.h>

#include <tlQt/Util.h>

#include <QBoxLayout>
#include <QHeaderView>
#include <QSettings>
#include <QTreeView>

namespace tl
{
    namespace play
    {
        struct InfoTool::Private
        {
            InfoModel* infoModel = nullptr;
            QTreeView* treeView = nullptr;
        };

        InfoTool::InfoTool(QWidget* parent) :
            ToolWidget(parent),
            _p(new Private)
        {
            TLRENDER_P();

            p.infoModel = new InfoModel(this);

            p.treeView = new QTreeView;
            p.treeView->setAllColumnsShowFocus(true);
            p.treeView->setAlternatingRowColors(true);
            p.treeView->setSelectionMode(QAbstractItemView::NoSelection);
            p.treeView->setIndentation(0);
            p.treeView->setModel(p.infoModel);

            auto layout = new QVBoxLayout;
            layout->setContentsMargins(0, 0, 0, 0);
            layout->setSpacing(0);
            layout->addWidget(p.treeView);
            auto widget = new QWidget;
            widget->setLayout(layout);
            addWidget(widget);

            QSettings settings;
            auto ba = settings.value(qt::versionedSettingsKey("InfoTool/Header")).toByteArray();
            if (!ba.isEmpty())
            {
                p.treeView->header()->restoreState(ba);
            }
        }

        InfoTool::~InfoTool()
        {
            TLRENDER_P();
            QSettings settings;
            settings.setValue(qt::versionedSettingsKey("InfoTool/Header"), p.treeView->header()->saveState());
        }

        void InfoTool::setInfo(const avio::Info& value)
        {
            TLRENDER_P();
            p.infoModel->setInfo(value);
        }
    }
}
