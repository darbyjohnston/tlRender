// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2024 Darby Johnston
// All rights reserved.

#include "MenuBar.h"

#include "FileActions.h"
#include "PlaybackActions.h"
#include "ViewActions.h"
#include "WindowActions.h"

#include "App.h"

namespace tl
{
    namespace examples
    {
        namespace player
        {
            void MenuBar::_init(
                const std::shared_ptr<dtk::Context>& context,
                const std::shared_ptr<FileActions>& fileActions,
                const std::shared_ptr<WindowActions>& windowActions,
                const std::shared_ptr<ViewActions>& viewActions,
                const std::shared_ptr<PlaybackActions>& playbackActions,
                const std::shared_ptr<IWidget>& parent)
            {
                dtk::MenuBar::_init(context, parent);

                auto fileMenu = dtk::Menu::create(context);
                auto actions = fileActions->getActions();
                fileMenu->addItem(actions["Open"]);
                fileMenu->addItem(actions["Close"]);
                fileMenu->addItem(actions["Reload"]);
                fileMenu->addDivider();
                fileMenu->addItem(actions["Exit"]);
                addMenu("File", fileMenu);

                auto windowMenu = dtk::Menu::create(context);
                actions = windowActions->getActions();
                windowMenu->addItem(actions["FullScreen"]);
                windowMenu->addDivider();
                windowMenu->addItem(actions["1920x1080"]);
                windowMenu->addItem(actions["3840x2160"]);
                addMenu("Window", windowMenu);

                auto viewMenu = dtk::Menu::create(context);
                actions = viewActions->getActions();
                viewMenu->addItem(actions["Frame"]);
                addMenu("View", viewMenu);

                auto playbackMenu = dtk::Menu::create(context);
                actions = playbackActions->getActions();
                playbackMenu->addItem(actions["Stop"]);
                playbackMenu->addItem(actions["Forward"]);
                playbackMenu->addItem(actions["Reverse"]);
                playbackMenu->addItem(actions["TogglePlayback"]);
                playbackMenu->addDivider();
                playbackMenu->addItem(actions["Start"]);
                playbackMenu->addItem(actions["Prev"]);
                playbackMenu->addItem(actions["Next"]);
                playbackMenu->addItem(actions["End"]);
                addMenu("Playback", playbackMenu);
            }

            MenuBar::~MenuBar()
            {}

            std::shared_ptr<MenuBar> MenuBar::create(
                const std::shared_ptr<dtk::Context>& context,
                const std::shared_ptr<FileActions>& fileActions,
                const std::shared_ptr<WindowActions>& windowActions,
                const std::shared_ptr<ViewActions>& viewActions,
                const std::shared_ptr<PlaybackActions>& playbackActions,
                const std::shared_ptr<IWidget>& parent)
            {
                auto out = std::shared_ptr<MenuBar>(new MenuBar);
                out->_init(context, fileActions, windowActions, viewActions, playbackActions, parent);
                return out;
            }
        }
    }
}
