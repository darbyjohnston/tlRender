// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2023 Darby Johnston
// All rights reserved.

#pragma once

#include <tlUI/Menu.h>

namespace tl
{
    namespace play_gl
    {
        class App;

        //! Audio menu.
        class AudioMenu : public ui::Menu
        {
            TLRENDER_NON_COPYABLE(AudioMenu);

        protected:
            void _init(
                const std::shared_ptr<App>&,
                const std::shared_ptr<system::Context>&);

            AudioMenu();

        public:
            ~AudioMenu();

            static std::shared_ptr<AudioMenu> create(
                const std::shared_ptr<App>&,
                const std::shared_ptr<system::Context>&);

        private:
            TLRENDER_PRIVATE();
        };
    }
}
