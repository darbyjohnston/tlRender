// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2025 Darby Johnston
// All rights reserved.

#include <tlPlayQtApp/FrameActions.h>

#include <tlPlayQtApp/App.h>

#include <tlQt/MetaTypes.h>
#include <tlQt/TimeObject.h>

#include <QActionGroup>

namespace tl
{
    namespace play_qt
    {
        struct FrameActions::Private
        {
            QSharedPointer<qt::TimelinePlayer> player;

            QMap<QString, QAction*> actions;
            QMap<QString, QActionGroup*> actionGroups;

            QScopedPointer<QMenu> menu;
        };

        FrameActions::FrameActions(App* app, QObject* parent) :
            QObject(parent),
            _p(new Private)
        {
            DTK_P();

            p.actions["Start"] = new QAction(parent);
            p.actions["Start"]->setText(tr("Go To Start"));
            p.actions["Start"]->setIcon(QIcon(":/Icons/TimeStart.svg"));
            p.actions["Start"]->setShortcut(QKeySequence(Qt::Key_Home));
            p.actions["Start"]->setToolTip(tr("Go to the start frame"));
            p.actions["End"] = new QAction(parent);
            p.actions["End"]->setText(tr("Go To End"));
            p.actions["End"]->setIcon(QIcon(":/Icons/TimeEnd.svg"));
            p.actions["End"]->setShortcut(QKeySequence(Qt::Key_End));
            p.actions["End"]->setToolTip(tr("Go to the end frame"));
            p.actions["FramePrev"] = new QAction(parent);
            p.actions["FramePrev"]->setText(tr("Previous Frame"));
            p.actions["FramePrev"]->setIcon(QIcon(":/Icons/FramePrev.svg"));
            p.actions["FramePrev"]->setShortcut(QKeySequence(Qt::Key_Left));
            p.actions["FramePrev"]->setToolTip(tr("Go to the previous frame"));
            p.actions["FramePrevX10"] = new QAction(parent);
            p.actions["FramePrevX10"]->setText(tr("Previous Frame X10"));
            p.actions["FramePrevX10"]->setShortcut(QKeySequence(Qt::SHIFT | Qt::Key_Left));
            p.actions["FramePrevX100"] = new QAction(parent);
            p.actions["FramePrevX100"]->setText(tr("Previous Frame X100"));
            p.actions["FramePrevX100"]->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_Left));
            p.actions["FrameNext"] = new QAction(parent);
            p.actions["FrameNext"]->setText(tr("Next Frame"));
            p.actions["FrameNext"]->setIcon(QIcon(":/Icons/FrameNext.svg"));
            p.actions["FrameNext"]->setShortcut(QKeySequence(Qt::Key_Right));
            p.actions["FrameNext"]->setToolTip(tr("Go to the next frame"));
            p.actions["FrameNextX10"] = new QAction(parent);
            p.actions["FrameNextX10"]->setText(tr("Next Frame X10"));
            p.actions["FrameNextX10"]->setShortcut(QKeySequence(Qt::SHIFT | Qt::Key_Right));
            p.actions["FrameNextX100"] = new QAction(parent);
            p.actions["FrameNextX100"]->setText(tr("Next Frame X100"));
            p.actions["FrameNextX100"]->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_Right));

            p.actions["FocusCurrentFrame"] = new QAction(parent);
            p.actions["FocusCurrentFrame"]->setText(tr("Focus Current Frame"));
            p.actions["FocusCurrentFrame"]->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_F));

            p.menu.reset(new QMenu);
            p.menu->setTitle(tr("&Frame"));
            p.menu->addAction(p.actions["Start"]);
            p.menu->addAction(p.actions["End"]);
            p.menu->addSeparator();
            p.menu->addAction(p.actions["FramePrev"]);
            p.menu->addAction(p.actions["FramePrevX10"]);
            p.menu->addAction(p.actions["FramePrevX100"]);
            p.menu->addAction(p.actions["FrameNext"]);
            p.menu->addAction(p.actions["FrameNextX10"]);
            p.menu->addAction(p.actions["FrameNextX100"]);
            p.menu->addSeparator();
            p.menu->addAction(p.actions["FocusCurrentFrame"]);

            _playerUpdate(app->player());
            _actionsUpdate();

            connect(
                p.actions["Start"],
                &QAction::triggered,
                [this]
                {
                    if (_p->player)
                    {
                        _p->player->start();
                    }
                });
            connect(
                p.actions["End"],
                &QAction::triggered,
                [this]
                {
                    if (_p->player)
                    {
                        _p->player->end();
                    }
                });
            connect(
                p.actions["FramePrev"],
                &QAction::triggered,
                [this]
                {
                    if (_p->player)
                    {
                        _p->player->framePrev();
                    }
                });
            connect(
                p.actions["FramePrevX10"],
                &QAction::triggered,
                [this]
                {
                    if (_p->player)
                    {
                        _p->player->timeAction(timeline::TimeAction::FramePrevX10);
                    }
                });
            connect(
                p.actions["FramePrevX100"],
                &QAction::triggered,
                [this]
                {
                    if (_p->player)
                    {
                        _p->player->timeAction(timeline::TimeAction::FramePrevX100);
                    }
                });
            connect(
                p.actions["FrameNext"],
                &QAction::triggered,
                [this]
                {
                    if (_p->player)
                    {
                        _p->player->frameNext();
                    }
                });
            connect(
                p.actions["FrameNextX10"],
                &QAction::triggered,
                [this]
                {
                    if (_p->player)
                    {
                        _p->player->timeAction(timeline::TimeAction::FrameNextX10);
                    }
                });
            connect(
                p.actions["FrameNextX100"],
                &QAction::triggered,
                [this]
                {
                    if (_p->player)
                    {
                        _p->player->timeAction(timeline::TimeAction::FrameNextX100);
                    }
                });

            connect(
                app,
                &App::playerChanged,
                [this](const QSharedPointer<qt::TimelinePlayer>& value)
                {
                    _playerUpdate(value);
                });
        }

        FrameActions::~FrameActions()
        {}

        const QMap<QString, QAction*>& FrameActions::actions() const
        {
            return _p->actions;
        }

        QMenu* FrameActions::menu() const
        {
            return _p->menu.get();
        }

        void FrameActions::_playerUpdate(const QSharedPointer<qt::TimelinePlayer>& player)
        {
            DTK_P();
            p.player = player;
            _actionsUpdate();
        }

        void FrameActions::_actionsUpdate()
        {
            DTK_P();
            QList<QString> keys = p.actions.keys();
            for (auto i : keys)
            {
                p.actions[i]->setEnabled(p.player.get());
            }
        }
    }
}
