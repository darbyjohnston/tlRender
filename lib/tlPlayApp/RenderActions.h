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

        //! Render actions.
        class RenderActions : public std::enable_shared_from_this<RenderActions>
        {
            DTK_NON_COPYABLE(RenderActions);

        protected:
            void _init(
                const std::shared_ptr<dtk::Context>&,
                const std::shared_ptr<App>&);

            RenderActions();

        public:
            ~RenderActions();

            static std::shared_ptr<RenderActions> create(
                const std::shared_ptr<dtk::Context>&,
                const std::shared_ptr<App>&);

            const std::vector<dtk::ImageType>& getColorBuffers() const;

            const std::map<std::string, std::shared_ptr<dtk::Action> >& getActions() const;

        private:
            DTK_PRIVATE();
        };
    }
}
