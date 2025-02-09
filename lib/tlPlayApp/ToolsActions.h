// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2025 Darby Johnston
// All rights reserved.

#pragma once

#include <dtk/ui/Action.h>

namespace tl
{
    namespace play_app
    {
        class App;

        //! Tools actions.
        class ToolsActions : public std::enable_shared_from_this<ToolsActions>
        {
            DTK_NON_COPYABLE(ToolsActions);

        protected:
            void _init(
                const std::shared_ptr<dtk::Context>&,
                const std::shared_ptr<App>&);

            ToolsActions();

        public:
            ~ToolsActions();

            static std::shared_ptr<ToolsActions> create(
                const std::shared_ptr<dtk::Context>&,
                const std::shared_ptr<App>&);

            const std::map<std::string, std::shared_ptr<dtk::Action> >& getActions() const;

        private:
            DTK_PRIVATE();
        };
    }
}
