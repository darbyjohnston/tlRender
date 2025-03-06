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
        struct TimelineActions::Private
        {
            std::map<std::string, std::shared_ptr<dtk::Action> > actions;
        };

        void TimelineActions::_init(
            const std::shared_ptr<dtk::Context>& context,
            const std::shared_ptr<App>& app,
            const std::shared_ptr<MainWindow>& mainWindow)
        {
            DTK_P();

            auto appWeak = std::weak_ptr<App>(app);
            p.actions["FrameView"] = std::make_shared<dtk::Action>(
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

            p.actions["ScrollToCurrentFrame"] = std::make_shared<dtk::Action>(
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

            p.actions["StopOnScrub"] = std::make_shared<dtk::Action>(
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

            p.actions["Thumbnails"] = std::make_shared<dtk::Action>(
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

            p.actions["Thumbnails100"] = std::make_shared<dtk::Action>(
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

            p.actions["Thumbnails200"] = std::make_shared<dtk::Action>(
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

            p.actions["Thumbnails300"] = std::make_shared<dtk::Action>(
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
    }
}
