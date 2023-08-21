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

            auto appWeak = std::weak_ptr<App>(app);
            p.actions["Editable"] = std::make_shared<ui::Action>(
                "Editable",
                [appWeak](bool value)
                {
                    if (auto app = appWeak.lock())
                    {
                        app->getMainWindow()->getTimelineWidget()->setEditable(value);
                    }
                });

            p.actions["EditAssociatedClips"] = std::make_shared<ui::Action>(
                "Edit Associated Clips",
                [appWeak](bool value)
                {
                    if (auto app = appWeak.lock())
                    {
                        auto mainWindow = app->getMainWindow();
                        auto options = mainWindow->getTimelineWidget()->getItemOptions();
                        options.editAssociatedClips = value;
                        mainWindow->getTimelineWidget()->setItemOptions(options);
                    }
                });

            p.actions["FrameView"] = std::make_shared<ui::Action>(
                "Frame Timeline View",
                [appWeak](bool value)
                {
                    if (auto app = appWeak.lock())
                    {
                        app->getMainWindow()->getTimelineWidget()->setFrameView(value);
                    }
                });

            p.actions["StopOnScrub"] = std::make_shared<ui::Action>(
                "Stop Playback When Scrubbing",
                [appWeak](bool value)
                {
                    if (auto app = appWeak.lock())
                    {
                        app->getMainWindow()->getTimelineWidget()->setStopOnScrub(value);
                    }
                });

            p.actions["Thumbnails"] = std::make_shared<ui::Action>(
                "Thumbnails",
                [appWeak](bool value)
                {
                    if (auto app = appWeak.lock())
                    {
                        auto mainWindow = app->getMainWindow();
                        auto options = mainWindow->getTimelineWidget()->getItemOptions();
                        options.thumbnails = value;
                        mainWindow->getTimelineWidget()->setItemOptions(options);
                    }
                });

            p.actions["Thumbnails100"] = std::make_shared<ui::Action>(
                "Small",
                [appWeak]
                {
                    if (auto app = appWeak.lock())
                    {
                        auto mainWindow = app->getMainWindow();
                        auto options = mainWindow->getTimelineWidget()->getItemOptions();
                        options.thumbnailHeight = 100;
                        options.waveformHeight = options.thumbnailHeight / 2;
                        mainWindow->getTimelineWidget()->setItemOptions(options);
                    }
                });

            p.actions["Thumbnails200"] = std::make_shared<ui::Action>(
                "Medium",
                [appWeak]
                {
                    if (auto app = appWeak.lock())
                    {
                        auto mainWindow = app->getMainWindow();
                        auto options = mainWindow->getTimelineWidget()->getItemOptions();
                        options.thumbnailHeight = 200;
                        options.waveformHeight = options.thumbnailHeight / 2;
                        mainWindow->getTimelineWidget()->setItemOptions(options);
                    }
                });

            p.actions["Thumbnails300"] = std::make_shared<ui::Action>(
                "Large",
                [appWeak]
                {
                    if (auto app = appWeak.lock())
                    {
                        auto mainWindow = app->getMainWindow();
                        auto options = mainWindow->getTimelineWidget()->getItemOptions();
                        options.thumbnailHeight = 300;
                        options.waveformHeight = options.thumbnailHeight / 2;
                        mainWindow->getTimelineWidget()->setItemOptions(options);
                    }
                });

            p.actions["Transitions"] = std::make_shared<ui::Action>(
                "Transitions",
                [appWeak](bool value)
                {
                    if (auto app = appWeak.lock())
                    {
                        auto mainWindow = app->getMainWindow();
                        auto options = mainWindow->getTimelineWidget()->getItemOptions();
                        options.showTransitions = value;
                        mainWindow->getTimelineWidget()->setItemOptions(options);
                    }
                });

            p.actions["Markers"] = std::make_shared<ui::Action>(
                "Markers",
                [appWeak](bool value)
                {
                    if (auto app = appWeak.lock())
                    {
                        auto mainWindow = app->getMainWindow();
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
