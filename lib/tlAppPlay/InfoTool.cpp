// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2022 Darby Johnston
// All rights reserved.

#include <tlAppPlay/InfoTool.h>

#include <tlAppPlay/App.h>
#include <tlAppPlay/InfoModel.h>
#include <tlAppPlay/SettingsObject.h>

#include <tlQt/Util.h>

#include <QBoxLayout>
#include <QHeaderView>
#include <QTreeView>

using namespace tl::core;

namespace tl
{
    namespace app
    {
        namespace play
        {
            struct InfoTool::Private
            {
                App* app = nullptr;
                InfoModel* infoModel = nullptr;
                QTreeView* treeView = nullptr;
            };

            InfoTool::InfoTool(
                App* app,
                QWidget* parent) :
                ToolWidget(parent),
                _p(new Private)
            {
                TLRENDER_P();

                p.app = app;
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

                app->settingsObject()->setDefaultValue("InfoTool/Header", QByteArray());
                auto ba = app->settingsObject()->value("InfoTool/Header").toByteArray();
                if (!ba.isEmpty())
                {
                    p.treeView->header()->restoreState(ba);
                }
            }

            InfoTool::~InfoTool()
            {
                TLRENDER_P();
                p.app->settingsObject()->setValue(
                    "InfoTool/Header",
                    p.treeView->header()->saveState());
            }

            void InfoTool::setInfo(const io::Info& value)
            {
                TLRENDER_P();
                p.infoModel->setInfo(value);
            }
        }
    }
}
