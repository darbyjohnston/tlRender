// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2025 Darby Johnston
// All rights reserved.

#pragma once

#include <dtk/ui/Action.h>
#include <dtk/ui/ToolButton.h>

namespace tl
{
    namespace play
    {
        //! Tool bar button.
        class ToolBarButton : public dtk::ToolButton
        {
            DTK_NON_COPYABLE(ToolBarButton);

        protected:
            void _init(
                const std::shared_ptr<dtk::Context>&,
                const std::shared_ptr<dtk::Action>&,
                const std::shared_ptr<IWidget>& parent);

            ToolBarButton();

        public:
            ~ToolBarButton();

            static std::shared_ptr<ToolBarButton> create(
                const std::shared_ptr<dtk::Context>&,
                const std::shared_ptr<dtk::Action>&,
                const std::shared_ptr<IWidget>& parent = nullptr);

        private:
            DTK_PRIVATE();
        };
    }
}
