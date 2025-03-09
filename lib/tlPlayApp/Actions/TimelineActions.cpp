// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2025 Darby Johnston
// All rights reserved.

#include <tlPlayApp/Actions/TimelineActions.h>

#include <tlPlayApp/App.h>

#include <tlTimelineUI/TimelineWidget.h>

#include <dtk/core/Format.h>

namespace tl
{
    namespace play
    {
        struct TimelineActions::Private
        {
            std::map<std::string, std::shared_ptr<dtk::Action> > actions;

            std::shared_ptr<dtk::ValueObserver<KeyShortcutsSettings> > keyShortcutsSettingsObserver;
        };

        void TimelineActions::_init(
            const std::shared_ptr<dtk::Context>& context,
            const std::shared_ptr<App>& app,
            const std::shared_ptr<MainWindow>& mainWindow)
        {
            DTK_P();

            auto appWeak = std::weak_ptr<App>(app);
            p.actions["FrameView"] = dtk::Action::create(
                "Frame Timeline View",
                [appWeak](bool value)
                {
                    if (auto app = appWeak.lock())
                    {
                        auto settings = app->getSettingsModel()->getTimeline();
                        settings.frameView = value;
                        app->getSettingsModel()->setTimeline(settings);
                    }
                });

            p.actions["Scroll"] = dtk::Action::create(
                "Scroll To Current Frame",
                [appWeak](bool value)
                {
                    if (auto app = appWeak.lock())
                    {
                        auto settings = app->getSettingsModel()->getTimeline();
                        settings.scroll = value;
                        app->getSettingsModel()->setTimeline(settings);
                    }
                });

            p.actions["StopOnScrub"] = dtk::Action::create(
                "Stop Playback When Scrubbing",
                [appWeak](bool value)
                {
                    if (auto app = appWeak.lock())
                    {
                        auto settings = app->getSettingsModel()->getTimeline();
                        settings.stopOnScrub = value;
                        app->getSettingsModel()->setTimeline(settings);
                    }
                });

            p.actions["Thumbnails"] = dtk::Action::create(
                "Thumbnails",
                [appWeak](bool value)
                {
                    if (auto app = appWeak.lock())
                    {
                        auto settings = app->getSettingsModel()->getTimeline();
                        settings.display.thumbnails = value;
                        app->getSettingsModel()->setTimeline(settings);
                    }
                });

            p.actions["ThumbnailsSmall"] = dtk::Action::create(
                "Small",
                [appWeak]
                {
                    if (auto app = appWeak.lock())
                    {
                        auto settings = app->getSettingsModel()->getTimeline();
                        settings.display.thumbnailHeight = 100;
                        settings.display.waveformHeight = settings.display.thumbnailHeight / 2;
                        app->getSettingsModel()->setTimeline(settings);
                    }
                });

            p.actions["ThumbnailsMedium"] = dtk::Action::create(
                "Medium",
                [appWeak]
                {
                    if (auto app = appWeak.lock())
                    {
                        auto settings = app->getSettingsModel()->getTimeline();
                        settings.display.thumbnailHeight = 200;
                        settings.display.waveformHeight = settings.display.thumbnailHeight / 2;
                        app->getSettingsModel()->setTimeline(settings);
                    }
                });

            p.actions["ThumbnailsLarge"] = dtk::Action::create(
                "Large",
                [appWeak]
                {
                    if (auto app = appWeak.lock())
                    {
                        auto settings = app->getSettingsModel()->getTimeline();
                        settings.display.thumbnailHeight = 300;
                        settings.display.waveformHeight = settings.display.thumbnailHeight / 2;
                        app->getSettingsModel()->setTimeline(settings);
                    }
                });

            p.keyShortcutsSettingsObserver = dtk::ValueObserver<KeyShortcutsSettings>::create(
                app->getSettingsModel()->observeKeyShortcuts(),
                [this](const KeyShortcutsSettings& value)
                {
                    _keyShortcutsUpdate(value);
                });
        }

        TimelineActions::TimelineActions() :
            _p(new Private)
        {}

        TimelineActions::~TimelineActions()
        {}

        std::shared_ptr<TimelineActions> TimelineActions::create(
            const std::shared_ptr<dtk::Context>& context,
            const std::shared_ptr<App>& app,
            const std::shared_ptr<MainWindow>& mainWindow)
        {
            auto out = std::shared_ptr<TimelineActions>(new TimelineActions);
            out->_init(context, app, mainWindow);
            return out;
        }

        const std::map<std::string, std::shared_ptr<dtk::Action> >& TimelineActions::getActions() const
        {
            return _p->actions;
        }

        void TimelineActions::_keyShortcutsUpdate(const KeyShortcutsSettings& value)
        {
            DTK_P();
            auto i = value.shortcuts.find("Timeline/FrameView");
            if (i != value.shortcuts.end())
            {
                p.actions["FrameView"]->setShortcut(i->second.key);
                p.actions["FrameView"]->setShortcutModifiers(i->second.modifiers);
                p.actions["FrameView"]->setTooltip(dtk::Format(
                    "Frame the timeline view.\n"
                    "\n"
                    "Shortcut: {0}").
                    arg(dtk::getShortcutLabel(i->second.key, i->second.modifiers)));
            }
            i = value.shortcuts.find("Timeline/Scroll");
            if (i != value.shortcuts.end())
            {
                p.actions["Scroll"]->setShortcut(i->second.key);
                p.actions["Scroll"]->setShortcutModifiers(i->second.modifiers);
                p.actions["Scroll"]->setTooltip(dtk::Format(
                    "Scroll the timeline view to the current frame.\n"
                    "\n"
                    "Shortcut: {0}").
                    arg(dtk::getShortcutLabel(i->second.key, i->second.modifiers)));
            }
            i = value.shortcuts.find("Timeline/StopOnScrub");
            if (i != value.shortcuts.end())
            {
                p.actions["StopOnScrub"]->setShortcut(i->second.key);
                p.actions["StopOnScrub"]->setShortcutModifiers(i->second.modifiers);
                p.actions["StopOnScrub"]->setTooltip(dtk::Format(
                    "Stop playback when scrubbing the timeline.\n"
                    "\n"
                    "Shortcut: {0}").
                    arg(dtk::getShortcutLabel(i->second.key, i->second.modifiers)));
            }
            i = value.shortcuts.find("Timeline/Thumbnails");
            if (i != value.shortcuts.end())
            {
                p.actions["Thumbnails"]->setShortcut(i->second.key);
                p.actions["Thumbnails"]->setShortcutModifiers(i->second.modifiers);
                p.actions["Thumbnails"]->setTooltip(dtk::Format(
                    "Toggle timeline thumbnails.\n"
                    "\n"
                    "Shortcut: {0}").
                    arg(dtk::getShortcutLabel(i->second.key, i->second.modifiers)));
            }
            i = value.shortcuts.find("Timeline/ThumbnailsSmall");
            if (i != value.shortcuts.end())
            {
                p.actions["ThumbnailsSmall"]->setShortcut(i->second.key);
                p.actions["ThumbnailsSmall"]->setShortcutModifiers(i->second.modifiers);
                p.actions["ThumbnailsSmall"]->setTooltip(dtk::Format(
                    "Small timeline thumbnails.\n"
                    "\n"
                    "Shortcut: {0}").
                    arg(dtk::getShortcutLabel(i->second.key, i->second.modifiers)));
            }
            i = value.shortcuts.find("Timeline/ThumbnailsMedium");
            if (i != value.shortcuts.end())
            {
                p.actions["ThumbnailsMedium"]->setShortcut(i->second.key);
                p.actions["ThumbnailsMedium"]->setShortcutModifiers(i->second.modifiers);
                p.actions["ThumbnailsMedium"]->setTooltip(dtk::Format(
                    "Medium timeline thumbnails.\n"
                    "\n"
                    "Shortcut: {0}").
                    arg(dtk::getShortcutLabel(i->second.key, i->second.modifiers)));
            }
            i = value.shortcuts.find("Timeline/ThumbnailsLarge");
            if (i != value.shortcuts.end())
            {
                p.actions["ThumbnailsLarge"]->setShortcut(i->second.key);
                p.actions["ThumbnailsLarge"]->setShortcutModifiers(i->second.modifiers);
                p.actions["ThumbnailsLarge"]->setTooltip(dtk::Format(
                    "Large timeline thumbnails.\n"
                    "\n"
                    "Shortcut: {0}").
                    arg(dtk::getShortcutLabel(i->second.key, i->second.modifiers)));
            }
        }
    }
}
