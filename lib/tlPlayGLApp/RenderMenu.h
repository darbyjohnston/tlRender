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

        //! Render menu.
        class RenderMenu : public ui::Menu
        {
            TLRENDER_NON_COPYABLE(RenderMenu);

        protected:
            void _init(
                const std::map<std::string, std::shared_ptr<ui::Action> >&,
                const std::shared_ptr<App>&,
                const std::shared_ptr<system::Context>&,
                const std::shared_ptr<IWidget>& parent);

            RenderMenu();

        public:
            ~RenderMenu();

            static std::shared_ptr<RenderMenu> create(
                const std::map<std::string, std::shared_ptr<ui::Action> >&,
                const std::shared_ptr<App>&,
                const std::shared_ptr<system::Context>&,
                const std::shared_ptr<IWidget>& parent = nullptr);

            void close() override;

        private:
            TLRENDER_PRIVATE();
        };
    }
}
