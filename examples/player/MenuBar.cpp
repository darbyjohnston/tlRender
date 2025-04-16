// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2024 Darby Johnston
// All rights reserved.

#include "MenuBar.h"

#include "App.h"

namespace tl
{
    namespace examples
    {
        namespace player
        {
            void MenuBar::_init(
                const std::shared_ptr<dtk::Context>& context,
                const std::map<std::string, std::shared_ptr<dtk::Action> >& fileActions,
                const std::map<std::string, std::shared_ptr<dtk::Action> >& playbackActions,
                const std::map<std::string, std::shared_ptr<dtk::Action> >& viewActions,
                const std::map<std::string, std::shared_ptr<dtk::Action> >& windowActions,
                const std::shared_ptr<IWidget>& parent)
            {
                dtk::MenuBar::_init(context, parent);

                auto fileMenu = dtk::Menu::create(context);
                auto tmp = fileActions;
                fileMenu->addItem(tmp["Open"]);
                fileMenu->addItem(tmp["Close"]);
                fileMenu->addItem(tmp["Reload"]);
                fileMenu->addDivider();
                fileMenu->addItem(tmp["Exit"]);
                addMenu("File", fileMenu);

                auto playbackMenu = dtk::Menu::create(context);
                tmp = playbackActions;
                playbackMenu->addItem(tmp["Stop"]);
                playbackMenu->addItem(tmp["Forward"]);
                playbackMenu->addItem(tmp["Reverse"]);
                addMenu("Playback", playbackMenu);
            }

            MenuBar::~MenuBar()
            {}

            std::shared_ptr<MenuBar> MenuBar::create(
                const std::shared_ptr<dtk::Context>& context,
                const std::map<std::string, std::shared_ptr<dtk::Action> >& fileActions,
                const std::map<std::string, std::shared_ptr<dtk::Action> >& playbackActions,
                const std::map<std::string, std::shared_ptr<dtk::Action> >& viewActions,
                const std::map<std::string, std::shared_ptr<dtk::Action> >& windowActions,
                const std::shared_ptr<IWidget>& parent)
            {
                auto out = std::shared_ptr<MenuBar>(new MenuBar);
                out->_init(context, fileActions, playbackActions, viewActions, windowActions, parent);
                return out;
            }
        }
    }
}
