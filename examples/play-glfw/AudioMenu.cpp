// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2023 Darby Johnston
// All rights reserved.

#include "AudioMenu.h"

#include "App.h"

namespace tl
{
    namespace examples
    {
        namespace play_glfw
        {
            struct AudioMenu::Private
            {
            };

            void AudioMenu::_init(
                const std::shared_ptr<App>& app,
                const std::shared_ptr<system::Context>& context)
            {
                Menu::_init(context);
                TLRENDER_P();
            }

            AudioMenu::AudioMenu() :
                _p(new Private)
            {}

            AudioMenu::~AudioMenu()
            {}

            std::shared_ptr<AudioMenu> AudioMenu::create(
                const std::shared_ptr<App>& app,
                const std::shared_ptr<system::Context>& context)
            {
                auto out = std::shared_ptr<AudioMenu>(new AudioMenu);
                out->_init(app, context);
                return out;
            }
        }
    }
}
