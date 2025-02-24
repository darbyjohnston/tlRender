// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2025 Darby Johnston
// All rights reserved.

#include <tlPlayApp/FrameMenu.h>

#include <tlPlayApp/App.h>
#include <tlPlayApp/MainWindow.h>

namespace tl
{
    namespace play_app
    {
        struct FrameMenu::Private
        {
            std::map<std::string, std::shared_ptr<dtk::Action> > actions;
        };

        void FrameMenu::_init(
            const std::shared_ptr<dtk::Context>& context,
            const std::shared_ptr<App>& app,
            const std::map<std::string, std::shared_ptr<dtk::Action> >& actions,
            const std::shared_ptr<IWidget>& parent)
        {
            Menu::_init(context, parent);
            DTK_P();

            p.actions = actions;

            addItem(p.actions["Start"]);
            addItem(p.actions["End"]);
            addDivider();
            addItem(p.actions["Prev"]);
            addItem(p.actions["PrevX10"]);
            addItem(p.actions["PrevX100"]);
            addDivider();
            addItem(p.actions["Next"]);
            addItem(p.actions["NextX10"]);
            addItem(p.actions["NextX100"]);
            addDivider();
            addItem(p.actions["FocusCurrent"]);
        }

        FrameMenu::FrameMenu() :
            _p(new Private)
        {}

        FrameMenu::~FrameMenu()
        {}

        std::shared_ptr<FrameMenu> FrameMenu::create(
            const std::shared_ptr<dtk::Context>& context,
            const std::shared_ptr<App>& app,
            const std::map<std::string, std::shared_ptr<dtk::Action> >& actions,
            const std::shared_ptr<IWidget>& parent)
        {
            auto out = std::shared_ptr<FrameMenu>(new FrameMenu);
            out->_init(context, app, actions, parent);
            return out;
        }
    }
}
