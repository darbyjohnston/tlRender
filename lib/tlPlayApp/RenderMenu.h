// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2025 Darby Johnston
// All rights reserved.

#pragma once

#include <dtk/ui/Menu.h>

namespace tl
{
    namespace play_app
    {
        class App;
        class RenderActions;

        //! Render menu.
        class RenderMenu : public dtk::Menu
        {
            DTK_NON_COPYABLE(RenderMenu);

        protected:
            void _init(
                const std::shared_ptr<dtk::Context>&,
                const std::shared_ptr<App>&,
                const std::shared_ptr<RenderActions>&,
                const std::shared_ptr<IWidget>& parent);

            RenderMenu();

        public:
            ~RenderMenu();

            static std::shared_ptr<RenderMenu> create(
                const std::shared_ptr<dtk::Context>&,
                const std::shared_ptr<App>&,
                const std::shared_ptr<RenderActions>&,
                const std::shared_ptr<IWidget>& parent = nullptr);

            void close() override;

        private:
            DTK_PRIVATE();
        };
    }
}
