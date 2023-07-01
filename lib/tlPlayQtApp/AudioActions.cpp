// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2023 Darby Johnston
// All rights reserved.

#include <tlPlayQtApp/AudioActions.h>

#include <tlPlayQtApp/App.h>

namespace tl
{
    namespace play_qt
    {
        struct AudioActions::Private
        {
            App* app = nullptr;

            QVector<QPointer<qt::TimelinePlayer> > timelinePlayers;

            QMap<QString, QAction*> actions;

            QMenu* menu = nullptr;
        };

        AudioActions::AudioActions(App* app, QObject* parent) :
            QObject(parent),
            _p(new Private)
        {
            TLRENDER_P();

            p.app = app;

            p.actions["IncreaseVolume"] = new QAction(this);
            p.actions["IncreaseVolume"]->setText(tr("Increase Volume"));
            p.actions["IncreaseVolume"]->setShortcut(QKeySequence(Qt::Key_Period));
            p.actions["DecreaseVolume"] = new QAction(this);
            p.actions["DecreaseVolume"]->setText(tr("Decrease Volume"));
            p.actions["DecreaseVolume"]->setShortcut(QKeySequence(Qt::Key_Comma));

            p.actions["Mute"] = new QAction(this);
            p.actions["Mute"]->setCheckable(true);
            p.actions["Mute"]->setText(tr("Mute"));
            QIcon muteIcon;
            muteIcon.addFile(":/Icons/Volume.svg", QSize(20, 20), QIcon::Normal, QIcon::Off);
            muteIcon.addFile(":/Icons/Mute.svg", QSize(20, 20), QIcon::Normal, QIcon::On);
            p.actions["Mute"]->setIcon(muteIcon);
            p.actions["Mute"]->setShortcut(QKeySequence(Qt::Key_M));
            p.actions["Mute"]->setToolTip(tr("Mute the audio"));

            p.menu = new QMenu;
            p.menu->setTitle(tr("&Audio"));
            p.menu->addAction(p.actions["IncreaseVolume"]);
            p.menu->addAction(p.actions["DecreaseVolume"]);
            p.menu->addAction(p.actions["Mute"]);

            _actionsUpdate();

            connect(
                p.actions["Mute"],
                &QAction::toggled,
                app,
                &App::setMute);

            connect(
                app,
                &App::muteChanged,
                [this](bool)
                {
                    _actionsUpdate();
                });
        }

        AudioActions::~AudioActions()
        {}

        const QMap<QString, QAction*>& AudioActions::actions() const
        {
            return _p->actions;
        }

        QMenu* AudioActions::menu() const
        {
            return _p->menu;
        }

        void AudioActions::setTimelinePlayers(const QVector<QPointer<qt::TimelinePlayer> >& timelinePlayers)
        {
            TLRENDER_P();
            if (!p.timelinePlayers.empty())
            {
                disconnect(
                    p.actions["IncreaseVolume"],
                    SIGNAL(triggered()),
                    this,
                    SLOT(_increaseVolumeCallback()));
                disconnect(
                    p.actions["DecreaseVolume"],
                    SIGNAL(triggered()),
                    this,
                    SLOT(_decreaseVolumeCallback()));
            }

            p.timelinePlayers = timelinePlayers;

            if (!p.timelinePlayers.empty())
            {
                connect(
                    p.actions["IncreaseVolume"],
                    SIGNAL(triggered()),
                    SLOT(_increaseVolumeCallback()));
                connect(
                    p.actions["DecreaseVolume"],
                    SIGNAL(triggered()),
                    SLOT(_decreaseVolumeCallback()));
            }

            _actionsUpdate();
        }

        void AudioActions::_increaseVolumeCallback()
        {
            TLRENDER_P();
            p.app->setVolume(p.app->volume() + .1F);
        }

        void AudioActions::_decreaseVolumeCallback()
        {
            TLRENDER_P();
            p.app->setVolume(p.app->volume() - .1F);
        }

        void AudioActions::_actionsUpdate()
        {
            TLRENDER_P();

            const size_t count = p.timelinePlayers.size();
            for (auto i : p.actions)
            {
                i->setEnabled(count > 0);
            }

            {
                QSignalBlocker blocker(p.actions["Mute"]);
                p.actions["Mute"]->setChecked(p.app->isMuted());
            }
        }
    }
}
