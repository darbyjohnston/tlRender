// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2022 Darby Johnston
// All rights reserved.

#include <tlPlay/WindowActions.h>

#include <tlPlay/App.h>

namespace tl
{
    namespace play
    {
        struct WindowActions::Private
        {
            App* app = nullptr;

            std::vector<qt::TimelinePlayer*> timelinePlayers;

            QMap<QString, QAction*> actions;

            QMenu* menu = nullptr;
        };

        WindowActions::WindowActions(App* app, QObject* parent) :
            QObject(parent),
            _p(new Private)
        {
            TLRENDER_P();

            p.app = app;

            p.actions["Resize1280x720"] = new QAction(this);
            p.actions["Resize1280x720"]->setText(tr("Resize 1280x720"));
            p.actions["Resize1920x1080"] = new QAction(this);
            p.actions["Resize1920x1080"]->setText(tr("Resize 1920x1080"));
            p.actions["Resize1920x1080"] = new QAction(this);
            p.actions["Resize1920x1080"]->setText(tr("Resize 1920x1080"));
            p.actions["FullScreen"] = new QAction(this);
            p.actions["FullScreen"]->setText(tr("Full Screen"));
            p.actions["FullScreen"]->setIcon(QIcon(":/Icons/WindowFullScreen.svg"));
            p.actions["FullScreen"]->setShortcut(QKeySequence(Qt::Key_U));
            p.actions["FullScreen"]->setToolTip(tr("Toggle full screen"));
            p.actions["FloatOnTop"] = new QAction(this);
            p.actions["FloatOnTop"]->setCheckable(true);
            p.actions["FloatOnTop"]->setText(tr("Float On Top"));
            p.actions["Secondary"] = new QAction(this);
            p.actions["Secondary"]->setCheckable(true);
            p.actions["Secondary"]->setText(tr("Secondary"));
            p.actions["Secondary"]->setIcon(QIcon(":/Icons/WindowSecondary.svg"));
            p.actions["Secondary"]->setShortcut(QKeySequence(Qt::Key_Y));
            p.actions["Secondary"]->setToolTip(tr("Toggle secondary window"));
            p.actions["SecondaryFloatOnTop"] = new QAction(this);
            p.actions["SecondaryFloatOnTop"]->setCheckable(true);
            p.actions["SecondaryFloatOnTop"]->setText(tr("Secondary Float On Top"));

            p.menu = new QMenu;
            p.menu->setTitle(tr("&Window"));
            p.menu->addAction(p.actions["Resize1280x720"]);
            p.menu->addAction(p.actions["Resize1920x1080"]);
            p.menu->addSeparator();
            p.menu->addAction(p.actions["FullScreen"]);
            p.menu->addAction(p.actions["FloatOnTop"]);
            p.menu->addSeparator();
            p.menu->addAction(p.actions["Secondary"]);
            p.menu->addAction(p.actions["SecondaryFloatOnTop"]);

            _actionsUpdate();
        }

        const QMap<QString, QAction*>& WindowActions::actions() const
        {
            return _p->actions;
        }

        QMenu* WindowActions::menu() const
        {
            return _p->menu;
        }

        void WindowActions::setTimelinePlayers(const std::vector<qt::TimelinePlayer*>& timelinePlayers)
        {
            TLRENDER_P();
            p.timelinePlayers = timelinePlayers;
            _actionsUpdate();
        }

        void WindowActions::_actionsUpdate()
        {
        }
    }
}
