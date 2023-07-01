// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2023 Darby Johnston
// All rights reserved.

#include <tlPlayQtApp/ViewActions.h>

#include <tlPlayQtApp/App.h>

namespace tl
{
    namespace play_qt
    {
        struct ViewActions::Private
        {
            App* app = nullptr;

            QMap<QString, QAction*> actions;

            QMenu* menu = nullptr;
        };

        ViewActions::ViewActions(App* app, QObject* parent) :
            QObject(parent),
            _p(new Private)
        {
            TLRENDER_P();

            p.app = app;

            p.actions["Frame"] = new QAction(this);
            p.actions["Frame"]->setCheckable(true);
            p.actions["Frame"]->setText(tr("Frame"));
            p.actions["Frame"]->setIcon(QIcon(":/Icons/ViewFrame.svg"));
            p.actions["Frame"]->setToolTip(tr("Frame the view to fit the window"));

            p.actions["Zoom1To1"] = new QAction(this);
            p.actions["Zoom1To1"]->setText(tr("Zoom 1:1"));
            p.actions["Zoom1To1"]->setIcon(QIcon(":/Icons/ViewZoom1To1.svg"));
            p.actions["Zoom1To1"]->setToolTip(tr("Set the view zoom to 1:1"));

            p.actions["ZoomIn"] = new QAction(this);
            p.actions["ZoomIn"]->setText(tr("Zoom In"));

            p.actions["ZoomOut"] = new QAction(this);
            p.actions["ZoomOut"]->setText(tr("Zoom Out"));

            p.menu = new QMenu;
            p.menu->setTitle(tr("&View"));
            p.menu->addAction(p.actions["Frame"]);
            p.menu->addAction(p.actions["Zoom1To1"]);
            p.menu->addAction(p.actions["ZoomIn"]);
            p.menu->addAction(p.actions["ZoomOut"]);

            _actionsUpdate();
        }

        ViewActions::~ViewActions()
        {}

        const QMap<QString, QAction*>& ViewActions::actions() const
        {
            return _p->actions;
        }

        QMenu* ViewActions::menu() const
        {
            return _p->menu;
        }

        void ViewActions::_actionsUpdate()
        {}
    }
}
