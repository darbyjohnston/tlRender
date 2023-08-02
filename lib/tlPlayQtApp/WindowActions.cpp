// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2023 Darby Johnston
// All rights reserved.

#include <tlPlayQtApp/WindowActions.h>

#include <tlPlayQtApp/App.h>

#include <tlQt/MetaTypes.h>

#include <QActionGroup>

namespace tl
{
    namespace play_qt
    {
        struct WindowActions::Private
        {
            App* app = nullptr;

            QMap<QString, QAction*> actions;
            QMap<QString, QActionGroup*> actionGroups;

            QMenu* menu = nullptr;
        };

        WindowActions::WindowActions(App* app, QObject* parent) :
            QObject(parent),
            _p(new Private)
        {
            TLRENDER_P();

            p.app = app;

            const std::vector<imaging::Size> sizes =
            {
                imaging::Size(1280, 720),
                imaging::Size(1280, 720),
                imaging::Size(1920, 1080)
            };
            for (const auto& i : sizes)
            {
                const QString key = QString("Resize/%1x%2").arg(i.w).arg(i.h);
                p.actions[key] = new QAction(parent);
                p.actions[key]->setData(QVariant::fromValue<imaging::Size>(i));
                p.actions[key]->setText(QString("%1x%2").arg(i.w).arg(i.h));
            }
            p.actionGroups["Resize"] = new QActionGroup(this);
            for (auto i : sizes)
            {
                const QString key = QString("Resize/%1x%2").arg(i.w).arg(i.h);
                p.actionGroups["Resize"]->addAction(p.actions[key]);
            }

            p.actions["FullScreen"] = new QAction(this);
            p.actions["FullScreen"]->setText(tr("Full Screen"));
            p.actions["FullScreen"]->setIcon(QIcon(":/Icons/WindowFullScreen.svg"));
            p.actions["FullScreen"]->setShortcut(QKeySequence(Qt::Key_U));
            p.actions["FullScreen"]->setToolTip(tr("Toggle the window full screen"));

            p.actions["FloatOnTop"] = new QAction(this);
            p.actions["FloatOnTop"]->setCheckable(true);
            p.actions["FloatOnTop"]->setText(tr("Float On Top"));

            p.actions["Secondary"] = new QAction(this);
            p.actions["Secondary"]->setCheckable(true);
            p.actions["Secondary"]->setText(tr("Secondary"));
            p.actions["Secondary"]->setIcon(QIcon(":/Icons/WindowSecondary.svg"));
            p.actions["Secondary"]->setShortcut(QKeySequence(Qt::Key_Y));
            p.actions["Secondary"]->setToolTip(tr("Toggle the secondary window"));

            p.actions["SecondaryFloatOnTop"] = new QAction(this);
            p.actions["SecondaryFloatOnTop"]->setCheckable(true);
            p.actions["SecondaryFloatOnTop"]->setText(tr("Secondary Float On Top"));

            p.menu = new QMenu;
            p.menu->setTitle(tr("&Window"));
            auto resizeMenu = p.menu->addMenu(tr("Resize"));
            for (auto i : sizes)
            {
                const QString key = QString("Resize/%1x%2").arg(i.w).arg(i.h);
                resizeMenu->addAction(p.actions[key]);
            }
            p.menu->addSeparator();
            p.menu->addAction(p.actions["FullScreen"]);
            p.menu->addAction(p.actions["FloatOnTop"]);
            p.menu->addSeparator();
            p.menu->addAction(p.actions["Secondary"]);
            p.menu->addAction(p.actions["SecondaryFloatOnTop"]);

            _actionsUpdate();

            connect(
                _p->actionGroups["Resize"],
                &QActionGroup::triggered,
                [this](QAction* action)
                {
                    Q_EMIT resize(action->data().value<imaging::Size>());
                });
        }

        WindowActions::~WindowActions()
        {}

        const QMap<QString, QAction*>& WindowActions::actions() const
        {
            return _p->actions;
        }

        QMenu* WindowActions::menu() const
        {
            return _p->menu;
        }

        void WindowActions::_actionsUpdate()
        {}
    }
}
