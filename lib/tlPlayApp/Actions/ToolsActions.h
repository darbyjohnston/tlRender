// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2025 Darby Johnston
// All rights reserved.

#pragma once

#include <tlPlayApp/Actions/IActions.h>

namespace tl
{
    namespace play
    {
        //! Tools actions.
        class ToolsActions : public IActions
        {
            DTK_NON_COPYABLE(ToolsActions);

        protected:
            void _init(
                const std::shared_ptr<dtk::Context>&,
                const std::shared_ptr<App>&);

            ToolsActions() = default;

        public:
            ~ToolsActions();

            static std::shared_ptr<ToolsActions> create(
                const std::shared_ptr<dtk::Context>&,
                const std::shared_ptr<App>&);
        };
    }
}
