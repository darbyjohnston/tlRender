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

        //! Compare actions.
        class CompareActions : public std::enable_shared_from_this<CompareActions>
        {
            DTK_NON_COPYABLE(CompareActions);

        protected:
            void _init(
                const std::shared_ptr<dtk::Context>&,
                const std::shared_ptr<App>&);

            CompareActions();

        public:
            ~CompareActions();

            static std::shared_ptr<CompareActions> create(
                const std::shared_ptr<dtk::Context>&,
                const std::shared_ptr<App>&);

            const std::map<std::string, std::shared_ptr<dtk::Action> >& getActions() const;

        private:
            void _keyShortcutsUpdate(const KeyShortcutsSettings&);

            DTK_PRIVATE();
        };
    }
}
