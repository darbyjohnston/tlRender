// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2025 Darby Johnston
// All rights reserved.

#pragma once

#include <tlPlayApp/Models/SettingsModel.h>

#include <dtk/ui/Action.h>

namespace tl
{
    namespace play
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

            const std::map<std::string, std::shared_ptr<dtk::Action> >& getActions() const;

        private:
            void _keyShortcutsUpdate(const KeyShortcutsSettings&);

            DTK_PRIVATE();
        };
    }
}
