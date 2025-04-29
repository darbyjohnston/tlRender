// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2025 Darby Johnston
// All rights reserved.

#include <tlPlayApp/Menus/AudioMenu.h>

#include <tlPlayApp/Actions/AudioActions.h>

namespace tl
{
    namespace play
    {
        void AudioMenu::_init(
            const std::shared_ptr<dtk::Context>& context,
            const std::shared_ptr<AudioActions>& audioActions,
            const std::shared_ptr<IWidget>& parent)
        {
            Menu::_init(context, parent);

            auto actions = audioActions->getActions();
            addAction(actions["VolumeUp"]);
            addAction(actions["VolumeDown"]);
            addAction(actions["Mute"]);
        }

        AudioMenu::~AudioMenu()
        {}

        std::shared_ptr<AudioMenu> AudioMenu::create(
            const std::shared_ptr<dtk::Context>& context,
            const std::shared_ptr<AudioActions>& actions,
            const std::shared_ptr<IWidget>& parent)
        {
            auto out = std::shared_ptr<AudioMenu>(new AudioMenu);
            out->_init(context, actions, parent);
            return out;
        }
    }
}
