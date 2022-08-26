// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2022 Darby Johnston
// All rights reserved.

#include <tlPlayApp/PlaybackActions.h>

#include <tlPlayApp/App.h>
#include <tlPlayApp/SettingsObject.h>

#include <tlQt/MetaTypes.h>
#include <tlQt/TimeObject.h>

#include <QActionGroup>

namespace tl
{
    namespace play
    {
        struct PlaybackActions::Private
        {
            App* app = nullptr;

            std::vector<qt::TimelinePlayer*> timelinePlayers;

            QMap<QString, QAction*> actions;
            QMap<QString, QActionGroup*> actionGroups;

            QMenu* menu = nullptr;
            QMenu* timeUnitsMenu = nullptr;
            QMenu* speedMenu = nullptr;
        };

        PlaybackActions::PlaybackActions(App* app, QObject* parent) :
            QObject(parent),
            _p(new Private)
        {
            TLRENDER_P();

            p.app = app;

            p.actions["Stop"] = new QAction(parent);
            p.actions["Stop"]->setData(QVariant::fromValue<timeline::Playback>(timeline::Playback::Stop));
            p.actions["Stop"]->setCheckable(true);
            p.actions["Stop"]->setText(tr("Stop Playback"));
            p.actions["Stop"]->setIcon(QIcon(":/Icons/PlaybackStop.svg"));
            p.actions["Stop"]->setShortcut(QKeySequence(Qt::Key_K));
            p.actions["Stop"]->setToolTip(tr("Stop playback"));
            p.actions["Forward"] = new QAction(parent);
            p.actions["Forward"]->setData(QVariant::fromValue<timeline::Playback>(timeline::Playback::Forward));
            p.actions["Forward"]->setCheckable(true);
            p.actions["Forward"]->setText(tr("Forward Playback"));
            p.actions["Forward"]->setIcon(QIcon(":/Icons/PlaybackForward.svg"));
            p.actions["Forward"]->setShortcut(QKeySequence(Qt::Key_L));
            p.actions["Forward"]->setToolTip(tr("Forward playback"));
            p.actions["Reverse"] = new QAction(parent);
            p.actions["Reverse"]->setData(QVariant::fromValue<timeline::Playback>(timeline::Playback::Reverse));
            p.actions["Reverse"]->setCheckable(true);
            p.actions["Reverse"]->setText(tr("Reverse Playback"));
            p.actions["Reverse"]->setIcon(QIcon(":/Icons/PlaybackReverse.svg"));
            p.actions["Reverse"]->setShortcut(QKeySequence(Qt::Key_J));
            p.actions["Reverse"]->setToolTip(tr("Reverse playback"));
            p.actionGroups["Playback"] = new QActionGroup(this);
            p.actionGroups["Playback"]->setExclusive(true);
            p.actionGroups["Playback"]->addAction(p.actions["Stop"]);
            p.actionGroups["Playback"]->addAction(p.actions["Forward"]);
            p.actionGroups["Playback"]->addAction(p.actions["Reverse"]);
            p.actions["Toggle"] = new QAction(parent);
            p.actions["Toggle"]->setText(tr("Toggle Playback"));
            p.actions["Toggle"]->setShortcut(QKeySequence(Qt::Key_Space));

            p.actions["Loop"] = new QAction(parent);
            p.actions["Loop"]->setData(QVariant::fromValue<timeline::Loop>(timeline::Loop::Loop));
            p.actions["Loop"]->setCheckable(true);
            p.actions["Loop"]->setText(tr("Loop Playback"));
            p.actions["Once"] = new QAction(parent);
            p.actions["Once"]->setData(QVariant::fromValue<timeline::Loop>(timeline::Loop::Once));
            p.actions["Once"]->setCheckable(true);
            p.actions["Once"]->setText(tr("Playback Once"));
            p.actions["PingPong"] = new QAction(parent);
            p.actions["PingPong"]->setData(QVariant::fromValue<timeline::Loop>(timeline::Loop::PingPong));
            p.actions["PingPong"]->setCheckable(true);
            p.actions["PingPong"]->setText(tr("Ping-Pong Playback"));
            p.actionGroups["Loop"] = new QActionGroup(this);
            p.actionGroups["Loop"]->setExclusive(true);
            p.actionGroups["Loop"]->addAction(p.actions["Loop"]);
            p.actionGroups["Loop"]->addAction(p.actions["Once"]);
            p.actionGroups["Loop"]->addAction(p.actions["PingPong"]);

            p.actions["Start"] = new QAction(parent);
            p.actions["Start"]->setText(tr("Go To Start"));
            p.actions["Start"]->setIcon(QIcon(":/Icons/TimeStart.svg"));
            p.actions["Start"]->setShortcut(QKeySequence(Qt::Key_Home));
            p.actions["Start"]->setToolTip(tr("Go to the start"));
            p.actions["End"] = new QAction(parent);
            p.actions["End"]->setText(tr("Go To End"));
            p.actions["End"]->setIcon(QIcon(":/Icons/TimeEnd.svg"));
            p.actions["End"]->setShortcut(QKeySequence(Qt::Key_End));
            p.actions["End"]->setToolTip(tr("Go to the end"));
            p.actions["FramePrev"] = new QAction(parent);
            p.actions["FramePrev"]->setText(tr("Previous Frame"));
            p.actions["FramePrev"]->setIcon(QIcon(":/Icons/FramePrev.svg"));
            p.actions["FramePrev"]->setShortcut(QKeySequence(Qt::Key_Left));
            p.actions["FramePrev"]->setToolTip(tr("Go to the previous frame"));
            p.actions["FramePrevX10"] = new QAction(parent);
            p.actions["FramePrevX10"]->setText(tr("Previous Frame X10"));
            p.actions["FramePrevX10"]->setShortcut(QKeySequence(Qt::SHIFT + Qt::Key_Left));
            p.actions["FramePrevX100"] = new QAction(parent);
            p.actions["FramePrevX100"]->setText(tr("Previous Frame X100"));
            p.actions["FramePrevX100"]->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_Left));
            p.actions["FrameNext"] = new QAction(parent);
            p.actions["FrameNext"]->setText(tr("Next Frame"));
            p.actions["FrameNext"]->setIcon(QIcon(":/Icons/FrameNext.svg"));
            p.actions["FrameNext"]->setShortcut(QKeySequence(Qt::Key_Right));
            p.actions["FrameNext"]->setToolTip(tr("Go to the next frame"));
            p.actions["FrameNextX10"] = new QAction(parent);
            p.actions["FrameNextX10"]->setText(tr("Next Frame X10"));
            p.actions["FrameNextX10"]->setShortcut(QKeySequence(Qt::SHIFT + Qt::Key_Right));
            p.actions["FrameNextX100"] = new QAction(parent);
            p.actions["FrameNextX100"]->setText(tr("Next Frame X100"));
            p.actions["FrameNextX100"]->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_Right));

            p.actions["SetInPoint"] = new QAction(parent);
            p.actions["SetInPoint"]->setText(tr("Set In Point"));
            p.actions["SetInPoint"]->setShortcut(QKeySequence(Qt::Key_I));
            p.actions["ResetInPoint"] = new QAction(parent);
            p.actions["ResetInPoint"]->setText(tr("Reset In Point"));
            p.actions["ResetInPoint"]->setShortcut(QKeySequence(Qt::SHIFT + Qt::Key_I));
            p.actions["SetOutPoint"] = new QAction(parent);
            p.actions["SetOutPoint"]->setText(tr("Set Out Point"));
            p.actions["SetOutPoint"]->setShortcut(QKeySequence(Qt::Key_O));
            p.actions["ResetOutPoint"] = new QAction(parent);
            p.actions["ResetOutPoint"]->setText(tr("Reset Out Point"));
            p.actions["ResetOutPoint"]->setShortcut(QKeySequence(Qt::SHIFT + Qt::Key_O));

            p.actions["FocusCurrentFrame"] = new QAction(parent);
            p.actions["FocusCurrentFrame"]->setText(tr("Focus Current Frame"));
            p.actions["FocusCurrentFrame"]->setShortcut(QKeySequence(Qt::Key_F));

            p.actions["Thumbnails"] = new QAction(parent);
            p.actions["Thumbnails"]->setCheckable(true);
            p.actions["Thumbnails"]->setText(tr("Timeline Thumbnails"));

            p.actions["StopOnScrub"] = new QAction(parent);
            p.actions["StopOnScrub"]->setCheckable(true);
            p.actions["StopOnScrub"]->setText(tr("Stop When Scrubbing"));

            p.actions["TimeUnits/Frames"] = new QAction(parent);
            p.actions["TimeUnits/Frames"]->setData(QVariant::fromValue<qt::TimeUnits>(qt::TimeUnits::Frames));
            p.actions["TimeUnits/Frames"]->setCheckable(true);
            p.actions["TimeUnits/Frames"]->setText(tr("Frames"));
            p.actions["TimeUnits/Seconds"] = new QAction(parent);
            p.actions["TimeUnits/Seconds"]->setData(QVariant::fromValue<qt::TimeUnits>(qt::TimeUnits::Seconds));
            p.actions["TimeUnits/Seconds"]->setCheckable(true);
            p.actions["TimeUnits/Seconds"]->setText(tr("Seconds"));
            p.actions["TimeUnits/Timecode"] = new QAction(parent);
            p.actions["TimeUnits/Timecode"]->setData(QVariant::fromValue<qt::TimeUnits>(qt::TimeUnits::Timecode));
            p.actions["TimeUnits/Timecode"]->setCheckable(true);
            p.actions["TimeUnits/Timecode"]->setText(tr("Timecode"));
            p.actionGroups["TimeUnits"] = new QActionGroup(this);
            p.actionGroups["TimeUnits"]->addAction(p.actions["TimeUnits/Frames"]);
            p.actionGroups["TimeUnits"]->addAction(p.actions["TimeUnits/Seconds"]);
            p.actionGroups["TimeUnits"]->addAction(p.actions["TimeUnits/Timecode"]);

            const QList<double> speeds =
            {
                1.0, 3.0, 6.0, 9.0, 12.0,
                16.0, 18.0, 23.98, 24.0, 29.97,
                30.0, 48.0, 59.94, 60.0, 120.0
            };
            for (auto i : speeds)
            {
                const QString key = QString("Speed/%1").arg(i);
                p.actions[key] = new QAction(parent);
                p.actions[key]->setData(i);
                p.actions[key]->setText(QString("%1").arg(i, 0, 'f', 2));
            }
            p.actions["Speed/Default"] = new QAction(parent);
            p.actions["Speed/Default"]->setData(0.F);
            p.actions["Speed/Default"]->setText(tr("Default"));
            p.actions["Speed/Default"]->setToolTip(tr("Default timeline speed"));
            p.actionGroups["Speed"] = new QActionGroup(this);
            for (auto i : speeds)
            {
                const QString key = QString("Speed/%1").arg(i);
                p.actionGroups["Speed"]->addAction(p.actions[key]);
            }
            p.actionGroups["Speed"]->addAction(p.actions["Speed/Default"]);

            p.menu = new QMenu;
            p.menu->setTitle(tr("&Playback"));
            p.menu->addAction(p.actions["Stop"]);
            p.menu->addAction(p.actions["Forward"]);
            p.menu->addAction(p.actions["Reverse"]);
            p.menu->addAction(p.actions["Toggle"]);
            p.menu->addSeparator();
            p.menu->addAction(p.actions["Loop"]);
            p.menu->addAction(p.actions["Once"]);
            p.menu->addAction(p.actions["PingPong"]);
            p.menu->addSeparator();
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
            p.menu->addAction(p.actions["SetInPoint"]);
            p.menu->addAction(p.actions["ResetInPoint"]);
            p.menu->addAction(p.actions["SetOutPoint"]);
            p.menu->addAction(p.actions["ResetOutPoint"]);
            p.menu->addSeparator();
            p.menu->addAction(p.actions["FocusCurrentFrame"]);
            p.menu->addSeparator();
            p.menu->addAction(p.actions["Thumbnails"]);
            p.menu->addAction(p.actions["StopOnScrub"]);

            p.timeUnitsMenu = new QMenu;
            p.timeUnitsMenu->addAction(p.actions["TimeUnits/Frames"]);
            p.timeUnitsMenu->addAction(p.actions["TimeUnits/Seconds"]);
            p.timeUnitsMenu->addAction(p.actions["TimeUnits/Timecode"]);

            p.speedMenu = new QMenu;
            for (auto i : speeds)
            {
                const QString key = QString("Speed/%1").arg(i);
                p.speedMenu->addAction(p.actions[key]);
            }
            p.speedMenu->addSeparator();
            p.speedMenu->addAction(p.actions["Speed/Default"]);

            _actionsUpdate();

            connect(
                p.actions["Toggle"],
                &QAction::triggered,
                [this]
                {
                    if (!_p->timelinePlayers.empty())
                    {
                        _p->timelinePlayers[0]->togglePlayback();
                    }
                });
            connect(
                p.actions["Start"],
                &QAction::triggered,
                [this]
                {
                    if (!_p->timelinePlayers.empty())
                    {
                        _p->timelinePlayers[0]->start();
                    }
                });
            connect(
                p.actions["End"],
                &QAction::triggered,
                [this]
                {
                    if (!_p->timelinePlayers.empty())
                    {
                        _p->timelinePlayers[0]->end();
                    }
                });
            connect(
                p.actions["FramePrev"],
                &QAction::triggered,
                [this]
                {
                    if (!_p->timelinePlayers.empty())
                    {
                        _p->timelinePlayers[0]->framePrev();
                    }
                });
            connect(
                p.actions["FramePrevX10"],
                &QAction::triggered,
                [this]
                {
                    if (!_p->timelinePlayers.empty())
                    {
                        _p->timelinePlayers[0]->timeAction(timeline::TimeAction::FramePrevX10);
                    }
                });
            connect(
                p.actions["FramePrevX100"],
                &QAction::triggered,
                [this]
                {
                    if (!_p->timelinePlayers.empty())
                    {
                        _p->timelinePlayers[0]->timeAction(timeline::TimeAction::FramePrevX100);
                    }
                });
            connect(
                p.actions["FrameNext"],
                &QAction::triggered,
                [this]
                {
                    if (!_p->timelinePlayers.empty())
                    {
                        _p->timelinePlayers[0]->frameNext();
                    }
                });
            connect(
                p.actions["FrameNextX10"],
                &QAction::triggered,
                [this]
                {
                    if (!_p->timelinePlayers.empty())
                    {
                        _p->timelinePlayers[0]->timeAction(timeline::TimeAction::FrameNextX10);
                    }
                });
            connect(
                p.actions["FrameNextX100"],
                &QAction::triggered,
                [this]
                {
                    if (!_p->timelinePlayers.empty())
                    {
                        _p->timelinePlayers[0]->timeAction(timeline::TimeAction::FrameNextX100);
                    }
                });
            connect(
                p.actions["Thumbnails"],
                &QAction::toggled,
                [app](bool value)
                {
                    app->settingsObject()->setValue("Timeline/Thumbnails", value);
                });
            connect(
                p.actions["StopOnScrub"],
                &QAction::toggled,
                [app](bool value)
                {
                    app->settingsObject()->setValue("Timeline/StopOnScrub", value);
                });

            connect(
                p.actionGroups["TimeUnits"],
                &QActionGroup::triggered,
                [app](QAction* action)
                {
                    app->timeObject()->setUnits(action->data().value<qt::TimeUnits>());
                });

            connect(
                _p->actionGroups["Speed"],
                &QActionGroup::triggered,
                [this](QAction* action)
                {
                    if (!_p->timelinePlayers.empty())
                    {
                        const float speed = action->data().toFloat();
                        _p->timelinePlayers[0]->setSpeed(speed > 0.F ? speed : _p->timelinePlayers[0]->defaultSpeed());
                    }
                });

            connect(
                p.actionGroups["Playback"],
                &QActionGroup::triggered,
                [this](QAction* action)
                {
                    if (!_p->timelinePlayers.empty())
                    {
                        _p->timelinePlayers[0]->setPlayback(action->data().value<timeline::Playback>());
                    }
                });

            connect(
                p.actionGroups["Loop"],
                &QActionGroup::triggered,
                [this](QAction* action)
                {
                    if (!_p->timelinePlayers.empty())
                    {
                        _p->timelinePlayers[0]->setLoop(action->data().value<timeline::Loop>());
                    }
                });
        }

        PlaybackActions::~PlaybackActions()
        {}

        const QMap<QString, QAction*>& PlaybackActions::actions() const
        {
            return _p->actions;
        }

        QMenu* PlaybackActions::menu() const
        {
            return _p->menu;
        }

        QMenu* PlaybackActions::timeUnitsMenu() const
        {
            return _p->timeUnitsMenu;
        }

        QMenu* PlaybackActions::speedMenu() const
        {
            return _p->speedMenu;
        }

        void PlaybackActions::setTimelinePlayers(const std::vector<qt::TimelinePlayer*>& timelinePlayers)
        {
            TLRENDER_P();
            if (!p.timelinePlayers.empty())
            {
                disconnect(
                    p.timelinePlayers[0],
                    SIGNAL(playbackChanged(tl::timeline::Playback)),
                    this,
                    SLOT(_playbackCallback(tl::timeline::Playback)));
                disconnect(
                    p.timelinePlayers[0],
                    SIGNAL(loopChanged(tl::timeline::Loop)),
                    this,
                    SLOT(_loopCallback(tl::timeline::Loop)));

                disconnect(
                    p.actions["SetInPoint"],
                    SIGNAL(triggered()),
                    p.timelinePlayers[0],
                    SLOT(setInPoint()));
                disconnect(
                    p.actions["ResetInPoint"],
                    SIGNAL(triggered()),
                    p.timelinePlayers[0],
                    SLOT(resetInPoint()));
                disconnect(
                    p.actions["SetOutPoint"],
                    SIGNAL(triggered()),
                    p.timelinePlayers[0],
                    SLOT(setOutPoint()));
                disconnect(
                    p.actions["ResetOutPoint"],
                    SIGNAL(triggered()),
                    p.timelinePlayers[0],
                    SLOT(resetOutPoint()));
            }

            p.timelinePlayers = timelinePlayers;

            if (!p.timelinePlayers.empty())
            {
                connect(
                    p.timelinePlayers[0],
                    SIGNAL(playbackChanged(tl::timeline::Playback)),
                    SLOT(_playbackCallback(tl::timeline::Playback)));
                connect(
                    p.timelinePlayers[0],
                    SIGNAL(loopChanged(tl::timeline::Loop)),
                    SLOT(_loopCallback(tl::timeline::Loop)));

                connect(
                    p.actions["SetInPoint"],
                    SIGNAL(triggered()),
                    p.timelinePlayers[0],
                    SLOT(setInPoint()));
                connect(
                    p.actions["ResetInPoint"],
                    SIGNAL(triggered()),
                    p.timelinePlayers[0],
                    SLOT(resetInPoint()));
                connect(
                    p.actions["SetOutPoint"],
                    SIGNAL(triggered()),
                    p.timelinePlayers[0],
                    SLOT(setOutPoint()));
                connect(
                    p.actions["ResetOutPoint"],
                    SIGNAL(triggered()),
                    p.timelinePlayers[0],
                    SLOT(resetOutPoint()));
            }

            _actionsUpdate();
        }

        void PlaybackActions::_playbackCallback(timeline::Playback value)
        {
            TLRENDER_P();
            const QSignalBlocker blocker(p.actionGroups["Playback"]);
            for (auto action : p.actionGroups["Playback"]->actions())
            {
                if (action->data().value<timeline::Playback>() == value)
                {
                    action->setChecked(true);
                    break;
                }
            }
        }

        void PlaybackActions::_loopCallback(timeline::Loop value)
        {
            TLRENDER_P();
            const QSignalBlocker blocker(p.actionGroups["Loop"]);
            for (auto action : p.actionGroups["Loop"]->actions())
            {
                if (action->data().value<timeline::Loop>() == value)
                {
                    action->setChecked(true);
                    break;
                }
            }
        }

        void PlaybackActions::_actionsUpdate()
        {
            TLRENDER_P();

            const size_t count = p.timelinePlayers.size();
            for (auto i : p.actions)
            {
                i->setEnabled(count > 0);
            }

            if (!p.timelinePlayers.empty())
            {
                {
                    QSignalBlocker blocker(p.actionGroups["Playback"]);
                    for (auto action : p.actionGroups["Playback"]->actions())
                    {
                        if (action->data().value<timeline::Playback>() == p.timelinePlayers[0]->playback())
                        {
                            action->setChecked(true);
                            break;
                        }
                    }
                }
                {
                    QSignalBlocker blocker(p.actionGroups["Loop"]);
                    for (auto action : p.actionGroups["Loop"]->actions())
                    {
                        if (action->data().value<timeline::Loop>() == p.timelinePlayers[0]->loop())
                        {
                            action->setChecked(true);
                            break;
                        }
                    }
                }
            }
            else
            {
                {
                    QSignalBlocker blocker(p.actionGroups["Playback"]);
                    p.actions["Stop"]->setChecked(true);
                }
                {
                    QSignalBlocker blocker(p.actionGroups["Loop"]);
                    p.actions["Loop"]->setChecked(true);
                }
            }

            {
                QSignalBlocker blocker(p.actionGroups["TimeUnits"]);
                for (auto action : p.actionGroups["TimeUnits"]->actions())
                {
                    if (action->data().value<qt::TimeUnits>() == p.app->timeObject()->units())
                    {
                        action->setChecked(true);
                        break;
                    }
                }
            }

            {
                QSignalBlocker blocker(p.actions["Thumbnails"]);
                p.actions["Thumbnails"]->setChecked(
                    p.app->settingsObject()->value("Timeline/Thumbnails").toBool());
            }

            {
                QSignalBlocker blocker(p.actions["StopOnScrub"]);
                p.actions["StopOnScrub"]->setChecked(
                    p.app->settingsObject()->value("Timeline/StopOnScrub").toBool());
            }
        }
    }
}
