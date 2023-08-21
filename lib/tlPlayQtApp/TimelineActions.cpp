// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2023 Darby Johnston
// All rights reserved.

#include <tlPlayQtApp/TimelineActions.h>

#include <tlPlayQtApp/App.h>
#include <tlPlayQtApp/SettingsObject.h>

#include <tlQt/MetaTypes.h>
#include <tlQt/TimeObject.h>

#include <QActionGroup>

namespace tl
{
    namespace play_qt
    {
        struct TimelineActions::Private
        {
            App* app = nullptr;

            QMap<QString, QAction*> actions;
            QMap<QString, QActionGroup*> actionGroups;

            QScopedPointer<QMenu> menu;
        };

        TimelineActions::TimelineActions(App* app, QObject* parent) :
            QObject(parent),
            _p(new Private)
        {
            TLRENDER_P();

            p.app = app;

            p.actions["Editable"] = new QAction(parent);
            p.actions["Editable"]->setCheckable(true);
            p.actions["Editable"]->setText(tr("Editable"));

            p.actions["FrameView"] = new QAction(parent);
            p.actions["FrameView"]->setCheckable(true);
            p.actions["FrameView"]->setText(tr("Frame Timeline View"));

            p.actions["StopOnScrub"] = new QAction(parent);
            p.actions["StopOnScrub"]->setCheckable(true);
            p.actions["StopOnScrub"]->setText(tr("Stop Playback When Scrubbing"));

            p.actions["Thumbnails"] = new QAction(parent);
            p.actions["Thumbnails"]->setCheckable(true);
            p.actions["Thumbnails"]->setText(tr("Thumbnails"));
            p.actions["ThumbnailsSize/Small"] = new QAction(this);
            p.actions["ThumbnailsSize/Small"]->setData(100);
            p.actions["ThumbnailsSize/Small"]->setCheckable(true);
            p.actions["ThumbnailsSize/Small"]->setText(tr("Small"));
            p.actions["ThumbnailsSize/Medium"] = new QAction(this);
            p.actions["ThumbnailsSize/Medium"]->setData(200);
            p.actions["ThumbnailsSize/Medium"]->setCheckable(true);
            p.actions["ThumbnailsSize/Medium"]->setText(tr("Medium"));
            p.actions["ThumbnailsSize/Large"] = new QAction(this);
            p.actions["ThumbnailsSize/Large"]->setData(300);
            p.actions["ThumbnailsSize/Large"]->setCheckable(true);
            p.actions["ThumbnailsSize/Large"]->setText(tr("Large"));
            p.actionGroups["ThumbnailsSize"] = new QActionGroup(this);
            p.actionGroups["ThumbnailsSize"]->addAction(p.actions["ThumbnailsSize/Small"]);
            p.actionGroups["ThumbnailsSize"]->addAction(p.actions["ThumbnailsSize/Medium"]);
            p.actionGroups["ThumbnailsSize"]->addAction(p.actions["ThumbnailsSize/Large"]);

            p.actions["Transitions"] = new QAction(parent);
            p.actions["Transitions"]->setCheckable(true);
            p.actions["Transitions"]->setText(tr("Transitions"));

            p.actions["Markers"] = new QAction(parent);
            p.actions["Markers"]->setCheckable(true);
            p.actions["Markers"]->setText(tr("Markers"));

            p.menu.reset(new QMenu);
            p.menu->setTitle(tr("&Timeline"));
            p.menu->addAction(p.actions["Editable"]);
            p.menu->addAction(p.actions["FrameView"]);
            p.menu->addAction(p.actions["StopOnScrub"]);
            p.menu->addAction(p.actions["Thumbnails"]);
            auto thumbnailsSizeMenu = p.menu->addMenu(tr("Thumbnails Size"));
            thumbnailsSizeMenu->addAction(p.actions["ThumbnailsSize/Small"]);
            thumbnailsSizeMenu->addAction(p.actions["ThumbnailsSize/Medium"]);
            thumbnailsSizeMenu->addAction(p.actions["ThumbnailsSize/Large"]);
            p.menu->addAction(p.actions["Transitions"]);
            p.menu->addAction(p.actions["Markers"]);

            _actionsUpdate();

            connect(
                p.actions["Editable"],
                &QAction::toggled,
                [app](bool value)
                {
                    app->settingsObject()->setValue("Timeline/Editable", value);
                });

            connect(
                p.actions["FrameView"],
                &QAction::toggled,
                [app](bool value)
                {
                    app->settingsObject()->setValue("Timeline/FrameView", value);
                });

            connect(
                p.actions["StopOnScrub"],
                &QAction::toggled,
                [app](bool value)
                {
                    app->settingsObject()->setValue("Timeline/StopOnScrub", value);
                });

            connect(
                p.actions["Thumbnails"],
                &QAction::toggled,
                [app](bool value)
                {
                    app->settingsObject()->setValue("Timeline/Thumbnails", value);
                });

            connect(
                p.actionGroups["ThumbnailsSize"],
                &QActionGroup::triggered,
                [app](QAction* action)
                {
                    const int value = action->data().toInt();
                    app->settingsObject()->setValue("Timeline/ThumbnailsSize", value);
                });

            connect(
                p.actions["Transitions"],
                &QAction::toggled,
                [app](bool value)
                {
                    app->settingsObject()->setValue("Timeline/Transitions", value);
                });

            connect(
                p.actions["Markers"],
                &QAction::toggled,
                [app](bool value)
                {
                    app->settingsObject()->setValue("Timeline/Markers", value);
                });
        }

        TimelineActions::~TimelineActions()
        {}

        const QMap<QString, QAction*>& TimelineActions::actions() const
        {
            return _p->actions;
        }

        QMenu* TimelineActions::menu() const
        {
            return _p->menu.get();
        }

        void TimelineActions::_actionsUpdate()
        {
            TLRENDER_P();
            {
                QSignalBlocker blocker(p.actions["Editable"]);
                p.actions["Editable"]->setChecked(
                    p.app->settingsObject()->value("Timeline/Editable").toBool());
            }
            {
                QSignalBlocker blocker(p.actions["FrameView"]);
                p.actions["FrameView"]->setChecked(
                    p.app->settingsObject()->value("Timeline/FrameView").toBool());
            }
            {
                QSignalBlocker blocker(p.actions["StopOnScrub"]);
                p.actions["StopOnScrub"]->setChecked(
                    p.app->settingsObject()->value("Timeline/StopOnScrub").toBool());
            }
            {
                QSignalBlocker blocker(p.actions["Thumbnails"]);
                p.actions["Thumbnails"]->setChecked(
                    p.app->settingsObject()->value("Timeline/Thumbnails").toBool());
            }
            {
                QSignalBlocker blocker(p.actionGroups["ThumbnailsSize"]);
                p.actions["ThumbnailsSize/Small"]->setChecked(false);
                p.actions["ThumbnailsSize/Medium"]->setChecked(false);
                p.actions["ThumbnailsSize/Large"]->setChecked(false);
                for (auto action : p.actionGroups["ThumbnailsSize"]->actions())
                {
                    if (action->data().toInt() ==
                        p.app->settingsObject()->value("Timeline/ThumbnailsSize").toInt())
                    {
                        action->setChecked(true);
                        break;
                    }
                }
            }
            {
                QSignalBlocker blocker(p.actions["Transitions"]);
                p.actions["Transitions"]->setChecked(
                    p.app->settingsObject()->value("Timeline/Transitions").toBool());
            }
            {
                QSignalBlocker blocker(p.actions["Markers"]);
                p.actions["Markers"]->setChecked(
                    p.app->settingsObject()->value("Timeline/Markers").toBool());
            }
        }
    }
}
