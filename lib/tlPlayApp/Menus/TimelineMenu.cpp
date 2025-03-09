// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2025 Darby Johnston
// All rights reserved.

#include <tlPlayApp/Menus/TimelineMenu.h>

#include <tlPlayApp/Actions/TimelineActions.h>
#include <tlPlayApp/App.h>
#include <tlPlayApp/MainWindow.h>

#include <tlTimelineUI/TimelineWidget.h>

namespace tl
{
    namespace play
    {
        struct TimelineMenu::Private
        {
            std::weak_ptr<MainWindow> mainWindow;

            std::map<std::string, std::shared_ptr<dtk::Action> > actions;
            std::map<int, std::shared_ptr<dtk::Action> > thumbnailsSizeItems;
            std::map<std::string, std::shared_ptr<dtk::Menu> > menus;

            std::shared_ptr<dtk::ValueObserver<bool> > frameViewObserver;
            std::shared_ptr<dtk::ValueObserver<bool> > scrollToCurrentFrameObserver;
            std::shared_ptr<dtk::ValueObserver<bool> > stopOnScrubObserver;
            std::shared_ptr<dtk::ValueObserver<timelineui::DisplayOptions> > displayOptionsObserver;
        };

        void TimelineMenu::_init(
            const std::shared_ptr<dtk::Context>& context,
            const std::shared_ptr<App>& app,
            const std::shared_ptr<MainWindow>& mainWindow,
            const std::shared_ptr<TimelineActions>& actions,
            const std::shared_ptr<IWidget>& parent)
        {
            Menu::_init(context, parent);
            DTK_P();

            p.mainWindow = mainWindow;

            p.actions = actions->getActions();

            addItem(p.actions["FrameView"]);
            addItem(p.actions["Scroll"]);
            addItem(p.actions["StopOnScrub"]);
            addItem(p.actions["Thumbnails"]);
            p.menus["ThumbnailSize"] = addSubMenu("Thumbnails Size");
            p.menus["ThumbnailSize"]->addItem(p.actions["ThumbnailsSmall"]);
            p.menus["ThumbnailSize"]->addItem(p.actions["ThumbnailsMedium"]);
            p.menus["ThumbnailSize"]->addItem(p.actions["ThumbnailsLarge"]);

            p.thumbnailsSizeItems[100] = p.actions["ThumbnailsSmall"];
            p.thumbnailsSizeItems[200] = p.actions["ThumbnailsMedium"];
            p.thumbnailsSizeItems[300] = p.actions["ThumbnailsLarge"];

            _thumbnailsSizeUpdate();

            p.frameViewObserver = dtk::ValueObserver<bool>::create(
                mainWindow->getTimelineWidget()->observeFrameView(),
                [this](bool value)
                {
                    setItemChecked(_p->actions["FrameView"], value);
                });

            p.scrollToCurrentFrameObserver = dtk::ValueObserver<bool>::create(
                mainWindow->getTimelineWidget()->observeScrollToCurrentFrame(),
                [this](bool value)
                {
                    setItemChecked(_p->actions["Scroll"], value);
                });

            p.stopOnScrubObserver = dtk::ValueObserver<bool>::create(
                mainWindow->getTimelineWidget()->observeStopOnScrub(),
                [this](bool value)
                {
                    setItemChecked(_p->actions["StopOnScrub"], value);
                });

            p.displayOptionsObserver = dtk::ValueObserver<timelineui::DisplayOptions>::create(
                mainWindow->getTimelineWidget()->observeDisplayOptions(),
                [this](const timelineui::DisplayOptions& value)
                {
                    setItemChecked(_p->actions["Thumbnails"], value.thumbnails);
                    _thumbnailsSizeUpdate();
                });
        }

        TimelineMenu::TimelineMenu() :
            _p(new Private)
        {}

        TimelineMenu::~TimelineMenu()
        {}

        std::shared_ptr<TimelineMenu> TimelineMenu::create(
            const std::shared_ptr<dtk::Context>& context,
            const std::shared_ptr<App>& app,
            const std::shared_ptr<MainWindow>& mainWindow,
            const std::shared_ptr<TimelineActions>& actions,
            const std::shared_ptr<IWidget>& parent)
        {
            auto out = std::shared_ptr<TimelineMenu>(new TimelineMenu);
            out->_init(context, app, mainWindow, actions, parent);
            return out;
        }

        void TimelineMenu::close()
        {
            Menu::close();
            DTK_P();
            for (const auto& menu : p.menus)
            {
                menu.second->close();
            }
        }

        void TimelineMenu::_thumbnailsSizeUpdate()
        {
            DTK_P();
            if (auto mainWindow = p.mainWindow.lock())
            {
                const auto options = mainWindow->getTimelineWidget()->getDisplayOptions();
                auto i = p.thumbnailsSizeItems.find(options.thumbnailHeight);
                if (i == p.thumbnailsSizeItems.end())
                {
                    i = p.thumbnailsSizeItems.begin();
                }
                for (auto item : p.thumbnailsSizeItems)
                {
                    const bool checked = item == *i;
                    p.menus["ThumbnailSize"]->setItemChecked(item.second, checked);
                }
            }
        }
    }
}
