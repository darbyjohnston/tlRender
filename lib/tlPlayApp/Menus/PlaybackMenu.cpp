// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2025 Darby Johnston
// All rights reserved.

#include <tlPlayApp/Menus/PlaybackMenu.h>

#include <tlPlayApp/Actions/PlaybackActions.h>

namespace tl
{
    namespace play
    {
        void PlaybackMenu::_init(
            const std::shared_ptr<dtk::Context>& context,
            const std::shared_ptr<PlaybackActions>& playbackActions,
            const std::shared_ptr<IWidget>& parent)
        {
            Menu::_init(context, parent);

            auto actions = playbackActions->getActions();
            addItem(actions["Stop"]);
            addItem(actions["Forward"]);
            addItem(actions["Reverse"]);
            addItem(actions["Toggle"]);
            addDivider();
            addItem(actions["JumpBack1s"]);
            addItem(actions["JumpBack10s"]);
            addItem(actions["JumpForward1s"]);
            addItem(actions["JumpForward10s"]);
            addDivider();
            addItem(actions["Loop"]);
            addItem(actions["Once"]);
            addItem(actions["PingPong"]);
            addDivider();
            addItem(actions["SetInPoint"]);
            addItem(actions["ResetInPoint"]);
            addItem(actions["SetOutPoint"]);
            addItem(actions["ResetOutPoint"]);
        }

        PlaybackMenu::~PlaybackMenu()
        {}

        std::shared_ptr<PlaybackMenu> PlaybackMenu::create(
            const std::shared_ptr<dtk::Context>& context,
            const std::shared_ptr<PlaybackActions>& playbackActions,
            const std::shared_ptr<IWidget>& parent)
        {
            auto out = std::shared_ptr<PlaybackMenu>(new PlaybackMenu);
            out->_init(context, playbackActions, parent);
            return out;
        }
    }
}
