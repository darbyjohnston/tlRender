// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2023 Darby Johnston
// All rights reserved.

#include <tlPlayGLApp/TimelineMenu.h>

#include <tlPlayGLApp/App.h>
#include <tlPlayGLApp/MainWindow.h>

#include <tlTimelineUI/TimelineWidget.h>

namespace tl
{
    namespace play_gl
    {
        struct TimelineMenu::Private
        {
            std::weak_ptr<MainWindow> mainWindow;

            std::map<std::string, std::shared_ptr<ui::Action> > actions;
            std::shared_ptr<Menu> thumbnailsSizeMenu;
            std::map<int, std::shared_ptr<ui::Action> > thumbnailsSizeItems;

            std::shared_ptr<observer::ValueObserver<bool> > editableObserver;
            std::shared_ptr<observer::ValueObserver<bool> > frameViewObserver;
            std::shared_ptr<observer::ValueObserver<bool> > stopOnScrubObserver;
            std::shared_ptr<observer::ValueObserver<timelineui::ItemOptions> > itemOptionsObserver;
        };

        void TimelineMenu::_init(
            const std::map<std::string, std::shared_ptr<ui::Action> >& actions,
            const std::shared_ptr<MainWindow>& mainWindow,
            const std::shared_ptr<App>& app,
            const std::shared_ptr<system::Context>& context,
            const std::shared_ptr<IWidget>& parent)
        {
            Menu::_init(context, parent);
            TLRENDER_P();

            p.mainWindow = mainWindow;

            p.actions = actions;
            addItem(p.actions["Editable"]);
            addItem(p.actions["FrameView"]);
            addItem(p.actions["StopOnScrub"]);
            addItem(p.actions["Thumbnails"]);
            p.thumbnailsSizeMenu = addSubMenu("Thumbnails Size");
            p.thumbnailsSizeMenu->addItem(p.actions["Thumbnails100"]);
            p.thumbnailsSizeMenu->addItem(p.actions["Thumbnails200"]);
            p.thumbnailsSizeMenu->addItem(p.actions["Thumbnails300"]);
            addItem(p.actions["Transitions"]);
            addItem(p.actions["Markers"]);

            p.thumbnailsSizeItems[100] = p.actions["Thumbnails100"];
            p.thumbnailsSizeItems[200] = p.actions["Thumbnails200"];
            p.thumbnailsSizeItems[300] = p.actions["Thumbnails300"];

            _thumbnailsSizeUpdate();

            p.editableObserver = observer::ValueObserver<bool>::create(
                mainWindow->getTimelineWidget()->observeEditable(),
                [this](bool value)
                {
                    setItemChecked(_p->actions["Editable"], value);
                });

            p.frameViewObserver = observer::ValueObserver<bool>::create(
                mainWindow->getTimelineWidget()->observeFrameView(),
                [this](bool value)
                {
                    setItemChecked(_p->actions["FrameView"], value);
                });

            p.stopOnScrubObserver = observer::ValueObserver<bool>::create(
                mainWindow->getTimelineWidget()->observeStopOnScrub(),
                [this](bool value)
                {
                    setItemChecked(_p->actions["StopOnScrub"], value);
                });

            p.itemOptionsObserver = observer::ValueObserver<timelineui::ItemOptions>::create(
                mainWindow->getTimelineWidget()->observeItemOptions(),
                [this](const timelineui::ItemOptions& value)
                {
                    setItemChecked(_p->actions["Thumbnails"], value.thumbnails);
                    _thumbnailsSizeUpdate();
                    setItemChecked(_p->actions["Transiitons"], value.showTransitions);
                    setItemChecked(_p->actions["Markers"], value.showMarkers);
                });
        }

        TimelineMenu::TimelineMenu() :
            _p(new Private)
        {}

        TimelineMenu::~TimelineMenu()
        {}

        std::shared_ptr<TimelineMenu> TimelineMenu::create(
            const std::map<std::string, std::shared_ptr<ui::Action> >& actions,
            const std::shared_ptr<MainWindow>& mainWindow,
            const std::shared_ptr<App>& app,
            const std::shared_ptr<system::Context>& context,
            const std::shared_ptr<IWidget>& parent)
        {
            auto out = std::shared_ptr<TimelineMenu>(new TimelineMenu);
            out->_init(actions, mainWindow, app, context, parent);
            return out;
        }

        void TimelineMenu::close()
        {
            Menu::close();
            TLRENDER_P();
            p.thumbnailsSizeMenu->close();
        }

        void TimelineMenu::_thumbnailsSizeUpdate()
        {
            TLRENDER_P();
            if (auto mainWindow = p.mainWindow.lock())
            {
                const auto options = mainWindow->getTimelineWidget()->getItemOptions();
                auto i = p.thumbnailsSizeItems.find(options.thumbnailHeight);
                if (i == p.thumbnailsSizeItems.end())
                {
                    i = p.thumbnailsSizeItems.begin();
                }
                for (auto item : p.thumbnailsSizeItems)
                {
                    const bool checked = item == *i;
                    p.thumbnailsSizeMenu->setItemChecked(item.second, checked);
                }
            }
        }
    }
}
