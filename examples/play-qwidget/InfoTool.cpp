// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2022 Darby Johnston
// All rights reserved.

#include "InfoTool.h"

#include <tlrQt/Util.h>

#include <QBoxLayout>
#include <QHeaderView>
#include <QSettings>

namespace tlr
{
    InfoTool::InfoTool(QWidget* parent) :
        ToolWidget(parent)
    {
        _infoModel = new InfoModel(this);

        _treeView = new QTreeView;
        _treeView->setAllColumnsShowFocus(true);
        _treeView->setAlternatingRowColors(true);
        _treeView->setSelectionMode(QAbstractItemView::NoSelection);
        _treeView->setIndentation(0);
        _treeView->setModel(_infoModel);

        auto layout = new QVBoxLayout;
        layout->setContentsMargins(0, 0, 0, 0);
        layout->setSpacing(0);
        layout->addWidget(_treeView);
        auto widget = new QWidget;
        widget->setLayout(layout);
        addWidget(widget);

        QSettings settings;
        auto ba = settings.value(qt::versionedSettingsKey("InfoTool/Header")).toByteArray();
        if (!ba.isEmpty())
        {
            _treeView->header()->restoreState(ba);
        }
    }

    InfoTool::~InfoTool()
    {
        QSettings settings;
        settings.setValue(qt::versionedSettingsKey("InfoTool/Header"), _treeView->header()->saveState());
    }

    void InfoTool::setInfo(const avio::Info& value)
    {
        _infoModel->setInfo(value);
    }
}
