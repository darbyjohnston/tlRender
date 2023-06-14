// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2023 Darby Johnston
// All rights reserved.

#include "PlaybackMenu.h"

#include "App.h"

namespace tl
{
    namespace examples
    {
        namespace play_glfw
        {
            struct PlaybackMenu::Private
            {
            };

            void PlaybackMenu::_init(
                const std::shared_ptr<App>& app,
                const std::shared_ptr<system::Context>& context)
            {
                Menu::_init(context);
                TLRENDER_P();
            }

            PlaybackMenu::PlaybackMenu() :
                _p(new Private)
            {}

            PlaybackMenu::~PlaybackMenu()
            {}

            std::shared_ptr<PlaybackMenu> PlaybackMenu::create(
                const std::shared_ptr<App>& app,
                const std::shared_ptr<system::Context>& context)
            {
                auto out = std::shared_ptr<PlaybackMenu>(new PlaybackMenu);
                out->_init(app, context);
                return out;
            }
        }
    }
}
