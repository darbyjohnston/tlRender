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

        //! Base class for action groups.
        class IActions : public std::enable_shared_from_this<IActions>
        {
            DTK_NON_COPYABLE(IActions);

        protected:
            void _init(
                const std::shared_ptr<dtk::Context>&,
                const std::shared_ptr<App>&,
                const std::string& name);

            IActions();

        public:
            ~IActions();

            const std::map<std::string, std::shared_ptr<dtk::Action> >& getActions() const;

        protected:
            void _keyShortcutsUpdate(const KeyShortcutsSettings&);

            std::string _name;
            std::map<std::string, std::shared_ptr<dtk::Action> > _actions;
            std::map<std::string, std::string> _tooltips;

        private:
            DTK_PRIVATE();
        };
    }
}
