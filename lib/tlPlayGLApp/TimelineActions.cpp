// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2023 Darby Johnston
// All rights reserved.

#include <tlPlayGLApp/TimelineActions.h>

#include <tlPlayGLApp/App.h>
#include <tlPlayGLApp/MainWindow.h>

#include <tlTimelineUI/TimelineWidget.h>

#include <tlCore/StringFormat.h>

namespace tl
{
    namespace play_gl
    {
        struct TimelineActions::Private
        {
            std::map<std::string, std::shared_ptr<ui::Action> > actions;
        };

        void TimelineActions::_init(
            const std::shared_ptr<MainWindow>& mainWindow,
            const std::shared_ptr<App>& app,
            const std::shared_ptr<system::Context>& context)
        {
            TLRENDER_P();

            auto mainWindowWeak = std::weak_ptr<MainWindow>(mainWindow);
            p.actions["Editable"] = std::make_shared<ui::Action>(
                "Editable",
                [mainWindowWeak](bool value)
                {
                    if (auto mainWindow = mainWindowWeak.lock())
                    {
                        mainWindow->getTimelineWidget()->setEditable(value);
                    }
                });

            p.actions["EditAssociatedClips"] = std::make_shared<ui::Action>(
                "Edit Associated Clips",
                [mainWindowWeak](bool value)
                {
                    if (auto mainWindow = mainWindowWeak.lock())
                    {
                        auto options = mainWindow->getTimelineWidget()->getItemOptions();
                        options.editAssociatedClips = value;
                        mainWindow->getTimelineWidget()->setItemOptions(options);
                    }
                });

            p.actions["FrameView"] = std::make_shared<ui::Action>(
                "Frame Timeline View",
                [mainWindowWeak](bool value)
                {
                    if (auto mainWindow = mainWindowWeak.lock())
                    {
                        mainWindow->getTimelineWidget()->setFrameView(value);
                    }
                });

            p.actions["StopOnScrub"] = std::make_shared<ui::Action>(
                "Stop Playback When Scrubbing",
                [mainWindowWeak](bool value)
                {
                    if (auto mainWindow = mainWindowWeak.lock())
                    {
                        mainWindow->getTimelineWidget()->setStopOnScrub(value);
                    }
                });

            p.actions["Thumbnails"] = std::make_shared<ui::Action>(
                "Thumbnails",
                [mainWindowWeak](bool value)
                {
                    if (auto mainWindow = mainWindowWeak.lock())
                    {
                        auto options = mainWindow->getTimelineWidget()->getItemOptions();
                        options.thumbnails = value;
                        mainWindow->getTimelineWidget()->setItemOptions(options);
                    }
                });

            p.actions["Thumbnails100"] = std::make_shared<ui::Action>(
                "Small",
                [mainWindowWeak]
                {
                    if (auto mainWindow = mainWindowWeak.lock())
                    {
                        auto options = mainWindow->getTimelineWidget()->getItemOptions();
                        options.thumbnailHeight = 100;
                        options.waveformHeight = options.thumbnailHeight / 2;
                        mainWindow->getTimelineWidget()->setItemOptions(options);
                    }
                });

            p.actions["Thumbnails200"] = std::make_shared<ui::Action>(
                "Medium",
                [mainWindowWeak]
                {
                    if (auto mainWindow = mainWindowWeak.lock())
                    {
                        auto options = mainWindow->getTimelineWidget()->getItemOptions();
                        options.thumbnailHeight = 200;
                        options.waveformHeight = options.thumbnailHeight / 2;
                        mainWindow->getTimelineWidget()->setItemOptions(options);
                    }
                });

            p.actions["Thumbnails300"] = std::make_shared<ui::Action>(
                "Large",
                [mainWindowWeak]
                {
                    if (auto mainWindow = mainWindowWeak.lock())
                    {
                        auto options = mainWindow->getTimelineWidget()->getItemOptions();
                        options.thumbnailHeight = 300;
                        options.waveformHeight = options.thumbnailHeight / 2;
                        mainWindow->getTimelineWidget()->setItemOptions(options);
                    }
                });

            p.actions["Transitions"] = std::make_shared<ui::Action>(
                "Transitions",
                [mainWindowWeak](bool value)
                {
                    if (auto mainWindow = mainWindowWeak.lock())
                    {
                        auto options = mainWindow->getTimelineWidget()->getItemOptions();
                        options.showTransitions = value;
                        mainWindow->getTimelineWidget()->setItemOptions(options);
                    }
                });

            p.actions["Markers"] = std::make_shared<ui::Action>(
                "Markers",
                [mainWindowWeak](bool value)
                {
                    if (auto mainWindow = mainWindowWeak.lock())
                    {
                        auto options = mainWindow->getTimelineWidget()->getItemOptions();
                        options.showMarkers = value;
                        mainWindow->getTimelineWidget()->setItemOptions(options);
                    }
                });
        }

        TimelineActions::TimelineActions() :
            _p(new Private)
        {}

        TimelineActions::~TimelineActions()
        {}

        std::shared_ptr<TimelineActions> TimelineActions::create(
            const std::shared_ptr<MainWindow>& mainWindow,
            const std::shared_ptr<App>& app,
            const std::shared_ptr<system::Context>& context)
        {
            auto out = std::shared_ptr<TimelineActions>(new TimelineActions);
            out->_init(mainWindow, app, context);
            return out;
        }

        const std::map<std::string, std::shared_ptr<ui::Action> >& TimelineActions::getActions() const
        {
            return _p->actions;
        }
    }
}
