// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2025 Darby Johnston
// All rights reserved.

#include <tlPlayApp/Actions/TimelineActions.h>

#include <tlPlayApp/App.h>

#include <tlTimelineUI/TimelineWidget.h>

namespace tl
{
    namespace play
    {
        void TimelineActions::_init(
            const std::shared_ptr<dtk::Context>& context,
            const std::shared_ptr<App>& app,
            const std::shared_ptr<MainWindow>& mainWindow)
        {
            IActions::_init(context, app, "Timeline");

            auto appWeak = std::weak_ptr<App>(app);
            _actions["FrameView"] = dtk::Action::create(
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

            _actions["Scroll"] = dtk::Action::create(
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

            _actions["StopOnScrub"] = dtk::Action::create(
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

            _actions["Thumbnails"] = dtk::Action::create(
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

            _actions["ThumbnailsSmall"] = dtk::Action::create(
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

            _actions["ThumbnailsMedium"] = dtk::Action::create(
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

            _actions["ThumbnailsLarge"] = dtk::Action::create(
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

            _tooltips =
            {
                { "FrameView", "Frame the timeline view." },
                { "Scroll", "Scroll the timeline view to the current frame." },
                { "StopOnScrub", "Stop playback when scrubbing the timeline." },
                { "Thumbnails", "Toggle timeline thumbnails." },
                { "ThumbnailsSmall", "Small timeline thumbnails." },
                { "ThumbnailsMedium", "Medium timeline thumbnails." },
                { "ThumbnailsLarge", "Large timeline thumbnails." }
            };

            _keyShortcutsUpdate(app->getSettingsModel()->getKeyShortcuts());
        }

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
    }
}
