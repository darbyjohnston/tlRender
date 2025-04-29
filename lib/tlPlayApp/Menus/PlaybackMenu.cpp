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
            addAction(actions["Stop"]);
            addAction(actions["Forward"]);
            addAction(actions["Reverse"]);
            addAction(actions["Toggle"]);
            addDivider();
            addAction(actions["JumpBack1s"]);
            addAction(actions["JumpBack10s"]);
            addAction(actions["JumpForward1s"]);
            addAction(actions["JumpForward10s"]);
            addDivider();
            addAction(actions["Loop"]);
            addAction(actions["Once"]);
            addAction(actions["PingPong"]);
            addDivider();
            addAction(actions["SetInPoint"]);
            addAction(actions["ResetInPoint"]);
            addAction(actions["SetOutPoint"]);
            addAction(actions["ResetOutPoint"]);
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
