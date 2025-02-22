// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2025 Darby Johnston
// All rights reserved.

#include <tlPlayApp/TimelineActions.h>

#include <tlPlayApp/App.h>
#include <tlPlayApp/MainWindow.h>

#include <tlTimelineUI/TimelineWidget.h>

namespace tl
{
    namespace play_app
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

            auto mainWindowWeak = std::weak_ptr<MainWindow>(mainWindow);
            p.actions["FrameView"] = std::make_shared<dtk::Action>(
                "Frame Timeline View",
                [mainWindowWeak](bool value)
                {
                    if (auto mainWindow = mainWindowWeak.lock())
                    {
                        mainWindow->getTimelineWidget()->setFrameView(value);
                    }
                });

            p.actions["ScrollToCurrentFrame"] = std::make_shared<dtk::Action>(
                "Scroll To Current Frame",
                [mainWindowWeak](bool value)
                {
                    if (auto mainWindow = mainWindowWeak.lock())
                    {
                        mainWindow->getTimelineWidget()->setScrollToCurrentFrame(value);
                    }
                });

            p.actions["StopOnScrub"] = std::make_shared<dtk::Action>(
                "Stop Playback When Scrubbing",
                [mainWindowWeak](bool value)
                {
                    if (auto mainWindow = mainWindowWeak.lock())
                    {
                        mainWindow->getTimelineWidget()->setStopOnScrub(value);
                    }
                });

            p.actions["Thumbnails"] = std::make_shared<dtk::Action>(
                "Thumbnails",
                [mainWindowWeak](bool value)
                {
                    if (auto mainWindow = mainWindowWeak.lock())
                    {
                        auto options = mainWindow->getTimelineWidget()->getDisplayOptions();
                        options.thumbnails = value;
                        mainWindow->getTimelineWidget()->setDisplayOptions(options);
                    }
                });

            p.actions["Thumbnails100"] = std::make_shared<dtk::Action>(
                "Small",
                [mainWindowWeak]
                {
                    if (auto mainWindow = mainWindowWeak.lock())
                    {
                        auto options = mainWindow->getTimelineWidget()->getDisplayOptions();
                        options.thumbnailHeight = 100;
                        options.waveformHeight = options.thumbnailHeight / 2;
                        mainWindow->getTimelineWidget()->setDisplayOptions(options);
                    }
                });

            p.actions["Thumbnails200"] = std::make_shared<dtk::Action>(
                "Medium",
                [mainWindowWeak]
                {
                    if (auto mainWindow = mainWindowWeak.lock())
                    {
                        auto options = mainWindow->getTimelineWidget()->getDisplayOptions();
                        options.thumbnailHeight = 200;
                        options.waveformHeight = options.thumbnailHeight / 2;
                        mainWindow->getTimelineWidget()->setDisplayOptions(options);
                    }
                });

            p.actions["Thumbnails300"] = std::make_shared<dtk::Action>(
                "Large",
                [mainWindowWeak]
                {
                    if (auto mainWindow = mainWindowWeak.lock())
                    {
                        auto options = mainWindow->getTimelineWidget()->getDisplayOptions();
                        options.thumbnailHeight = 300;
                        options.waveformHeight = options.thumbnailHeight / 2;
                        mainWindow->getTimelineWidget()->setDisplayOptions(options);
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
