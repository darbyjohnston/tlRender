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
        class MainWindow;

        //! Frame actions.
        class FrameActions : public std::enable_shared_from_this<FrameActions>
        {
            DTK_NON_COPYABLE(FrameActions);

        protected:
            void _init(
                const std::shared_ptr<dtk::Context>&,
                const std::shared_ptr<App>&,
                const std::shared_ptr<MainWindow>&);

            FrameActions();

        public:
            ~FrameActions();

            static std::shared_ptr<FrameActions> create(
                const std::shared_ptr<dtk::Context>&,
                const std::shared_ptr<App>&,
                const std::shared_ptr<MainWindow>&);

            const std::map<std::string, std::shared_ptr<dtk::Action> >& getActions() const;

        private:
            DTK_PRIVATE();
        };
    }
}

