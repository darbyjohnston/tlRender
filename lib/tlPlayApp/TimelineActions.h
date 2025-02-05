// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2025 Darby Johnston
// All rights reserved.

#pragma once

#include <tlUI/Action.h>

namespace tl
{
    namespace play_app
    {
        class App;
        class MainWindow;

        //! Timeline actions.
        class TimelineActions : public std::enable_shared_from_this<TimelineActions>
        {
            DTK_NON_COPYABLE(TimelineActions);

        protected:
            void _init(
                const std::shared_ptr<dtk::Context>&,
                const std::shared_ptr<App>&,
                const std::shared_ptr<MainWindow>&);

            TimelineActions();

        public:
            ~TimelineActions();

            static std::shared_ptr<TimelineActions> create(
                const std::shared_ptr<dtk::Context>&,
                const std::shared_ptr<App>&,
                const std::shared_ptr<MainWindow>&);

            const std::map<std::string, std::shared_ptr<ui::Action> >& getActions() const;

        private:
            DTK_PRIVATE();
        };
    }
}
