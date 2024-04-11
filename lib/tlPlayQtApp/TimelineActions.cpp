// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2024 Darby Johnston
// All rights reserved.

#include <tlPlayQtApp/TimelineActions.h>

#include <tlPlayQtApp/MainWindow.h>

#include <tlQtWidget/TimelineWidget.h>

#include <tlQt/MetaTypes.h>
#include <tlQt/TimeObject.h>

#include <QActionGroup>

namespace tl
{
    namespace play_qt
    {
        struct TimelineActions::Private
        {
            MainWindow* mainWindow = nullptr;

            QMap<QString, QAction*> actions;
            QMap<QString, QActionGroup*> actionGroups;

            QScopedPointer<QMenu> menu;
        };

        TimelineActions::TimelineActions(MainWindow* mainWindow, QObject* parent) :
            QObject(parent),
            _p(new Private)
        {
            TLRENDER_P();

            p.mainWindow = mainWindow;

            p.actions["Editable"] = new QAction(parent);
            p.actions["Editable"]->setCheckable(true);
            p.actions["Editable"]->setText(tr("Editable"));

            p.actions["EditAssociatedClips"] = new QAction(parent);
            p.actions["EditAssociatedClips"]->setCheckable(true);
            p.actions["EditAssociatedClips"]->setText(tr("Edit Associated Clips"));

            p.actions["FrameView"] = new QAction(parent);
            p.actions["FrameView"]->setCheckable(true);
            p.actions["FrameView"]->setText(tr("Frame Timeline View"));

            p.actions["ScrollPlayback"] = new QAction(parent);
            p.actions["ScrollPlayback"]->setCheckable(true);
            p.actions["ScrollPlayback"]->setText(tr("Scroll Playback"));

            p.actions["StopOnScrub"] = new QAction(parent);
            p.actions["StopOnScrub"]->setCheckable(true);
            p.actions["StopOnScrub"]->setText(tr("Stop Playback When Scrubbing"));

            p.actions["FirstTrack"] = new QAction(parent);
            p.actions["FirstTrack"]->setCheckable(true);
            p.actions["FirstTrack"]->setText(tr("First Track Only"));

            p.actions["TrackInfo"] = new QAction(parent);
            p.actions["TrackInfo"]->setCheckable(true);
            p.actions["TrackInfo"]->setText(tr("Track Information"));

            p.actions["ClipInfo"] = new QAction(parent);
            p.actions["ClipInfo"]->setCheckable(true);
            p.actions["ClipInfo"]->setText(tr("Clip Information"));

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
            p.menu->addAction(p.actions["EditAssociatedClips"]);
            p.menu->addSeparator();
            p.menu->addAction(p.actions["FrameView"]);
            p.menu->addAction(p.actions["ScrollPlayback"]);
            p.menu->addAction(p.actions["StopOnScrub"]);
            p.menu->addSeparator();
            p.menu->addAction(p.actions["FirstTrack"]);
            p.menu->addAction(p.actions["TrackInfo"]);
            p.menu->addAction(p.actions["ClipInfo"]);
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
                [mainWindow](bool value)
                {
                    mainWindow->timelineWidget()->setEditable(value);
                });

            connect(
                p.actions["EditAssociatedClips"],
                &QAction::toggled,
                [mainWindow](bool value)
                {
                    auto timelineWidget = mainWindow->timelineWidget();
                    auto itemOptions = timelineWidget->itemOptions();
                    itemOptions.editAssociatedClips = value;
                    timelineWidget->setItemOptions(itemOptions);
                });

            connect(
                p.actions["FrameView"],
                &QAction::toggled,
                [mainWindow](bool value)
                {
                    mainWindow->timelineWidget()->setFrameView(value);
                });

            connect(
                p.actions["ScrollPlayback"],
                &QAction::toggled,
                [mainWindow](bool value)
                {
                    mainWindow->timelineWidget()->setScrollPlayback(value);
                });

            connect(
                p.actions["StopOnScrub"],
                &QAction::toggled,
                [mainWindow](bool value)
                {
                    mainWindow->timelineWidget()->setStopOnScrub(value);
                });

            connect(
                p.actions["FirstTrack"],
                &QAction::toggled,
                [mainWindow](bool value)
                {
                    auto timelineWidget = mainWindow->timelineWidget();
                    auto itemOptions = timelineWidget->itemOptions();
                    itemOptions.tracks.clear();
                    if (value)
                    {
                        itemOptions.tracks.push_back(0);
                    }
                    timelineWidget->setItemOptions(itemOptions);
                });

            connect(
                p.actions["TrackInfo"],
                &QAction::toggled,
                [mainWindow](bool value)
                {
                    auto timelineWidget = mainWindow->timelineWidget();
                    auto itemOptions = timelineWidget->itemOptions();
                    itemOptions.trackInfo = value;
                    timelineWidget->setItemOptions(itemOptions);
                });

            connect(
                p.actions["ClipInfo"],
                &QAction::toggled,
                [mainWindow](bool value)
                {
                    auto timelineWidget = mainWindow->timelineWidget();
                    auto itemOptions = timelineWidget->itemOptions();
                    itemOptions.clipInfo = value;
                    timelineWidget->setItemOptions(itemOptions);
                });

            connect(
                p.actions["Thumbnails"],
                &QAction::toggled,
                [mainWindow](bool value)
                {
                    auto timelineWidget = mainWindow->timelineWidget();
                    auto itemOptions = timelineWidget->itemOptions();
                    itemOptions.thumbnails = value;
                    timelineWidget->setItemOptions(itemOptions);
                });

            connect(
                p.actionGroups["ThumbnailsSize"],
                &QActionGroup::triggered,
                [mainWindow](QAction* action)
                {
                    const int value = action->data().toInt();
                    auto timelineWidget = mainWindow->timelineWidget();
                    auto itemOptions = timelineWidget->itemOptions();
                    itemOptions.thumbnailHeight = value;
                    timelineWidget->setItemOptions(itemOptions);
                });

            connect(
                p.actions["Transitions"],
                &QAction::toggled,
                [mainWindow](bool value)
                {
                    auto timelineWidget = mainWindow->timelineWidget();
                    auto itemOptions = timelineWidget->itemOptions();
                    itemOptions.transitions = value;
                    timelineWidget->setItemOptions(itemOptions);
                });

            connect(
                p.actions["Markers"],
                &QAction::toggled,
                [mainWindow](bool value)
                {
                    auto timelineWidget = mainWindow->timelineWidget();
                    auto itemOptions = timelineWidget->itemOptions();
                    itemOptions.markers = value;
                    timelineWidget->setItemOptions(itemOptions);
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
                    p.mainWindow->timelineWidget()->isEditable());
            }
            {
                QSignalBlocker blocker(p.actions["EditAssociatedClips"]);
                const auto itemOptions = p.mainWindow->timelineWidget()->itemOptions();
                p.actions["EditAssociatedClips"]->setChecked(itemOptions.editAssociatedClips);
            }
            {
                QSignalBlocker blocker(p.actions["FrameView"]);
                p.actions["FrameView"]->setChecked(
                    p.mainWindow->timelineWidget()->hasFrameView());
            }
            {
                QSignalBlocker blocker(p.actions["ScrollPlayback"]);
                p.actions["ScrollPlayback"]->setChecked(
                    p.mainWindow->timelineWidget()->hasScrollPlayback());
            }
            {
                QSignalBlocker blocker(p.actions["StopOnScrub"]);
                p.actions["StopOnScrub"]->setChecked(
                    p.mainWindow->timelineWidget()->hasStopOnScrub());
            }
            {
                QSignalBlocker blocker(p.actions["FirstTrack"]);
                const auto itemOptions = p.mainWindow->timelineWidget()->itemOptions();
                p.actions["FirstTrack"]->setChecked(!itemOptions.tracks.empty());
            }
            {
                QSignalBlocker blocker(p.actions["TrackInfo"]);
                const auto itemOptions = p.mainWindow->timelineWidget()->itemOptions();
                p.actions["TrackInfo"]->setChecked(itemOptions.trackInfo);
            }
            {
                QSignalBlocker blocker(p.actions["ClipInfo"]);
                const auto itemOptions = p.mainWindow->timelineWidget()->itemOptions();
                p.actions["ClipInfo"]->setChecked(itemOptions.clipInfo);
            }
            {
                QSignalBlocker blocker(p.actions["Thumbnails"]);
                const auto itemOptions = p.mainWindow->timelineWidget()->itemOptions();
                p.actions["Thumbnails"]->setChecked(itemOptions.thumbnails);
            }
            {
                QSignalBlocker blocker(p.actionGroups["ThumbnailsSize"]);
                p.actions["ThumbnailsSize/Small"]->setChecked(false);
                p.actions["ThumbnailsSize/Medium"]->setChecked(false);
                p.actions["ThumbnailsSize/Large"]->setChecked(false);
                const auto itemOptions = p.mainWindow->timelineWidget()->itemOptions();
                for (auto action : p.actionGroups["ThumbnailsSize"]->actions())
                {
                    if (action->data().toInt() == itemOptions.thumbnailHeight)
                    {
                        action->setChecked(true);
                        break;
                    }
                }
            }
            {
                QSignalBlocker blocker(p.actions["Transitions"]);
                const auto itemOptions = p.mainWindow->timelineWidget()->itemOptions();
                p.actions["Transitions"]->setChecked(itemOptions.transitions);
            }
            {
                QSignalBlocker blocker(p.actions["Markers"]);
                const auto itemOptions = p.mainWindow->timelineWidget()->itemOptions();
                p.actions["Markers"]->setChecked(itemOptions.markers);
            }
        }
    }
}
