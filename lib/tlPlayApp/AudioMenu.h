// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2025 Darby Johnston
// All rights reserved.

#pragma once

#include <tlUI/Menu.h>

namespace tl
{
    namespace play_app
    {
        class App;

        //! Audio menu.
        class AudioMenu : public ui::Menu
        {
            DTK_NON_COPYABLE(AudioMenu);

        protected:
            void _init(
                const std::shared_ptr<dtk::Context>&,
                const std::shared_ptr<App>&,
                const std::map<std::string, std::shared_ptr<ui::Action> >&,
                const std::shared_ptr<IWidget>& parent);

            AudioMenu();

        public:
            ~AudioMenu();

            static std::shared_ptr<AudioMenu> create(
                const std::shared_ptr<dtk::Context>&,
                const std::shared_ptr<App>&,
                const std::map<std::string, std::shared_ptr<ui::Action> >&,
                const std::shared_ptr<IWidget>& parent = nullptr);

        private:
            DTK_PRIVATE();
        };
    }
}
