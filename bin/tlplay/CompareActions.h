// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2024 Darby Johnston
// All rights reserved.

#pragma once

#include <tlTimeline/Player.h>

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

            CompareActions() = default;

        public:
            ~CompareActions();

            static std::shared_ptr<CompareActions> create(
                const std::shared_ptr<dtk::Context>&,
                const std::shared_ptr<App>&);

            const std::map<std::string, std::shared_ptr<dtk::Action> >& getActions() const;

        private:
            std::map<std::string, std::shared_ptr<dtk::Action> > _actions;
            std::shared_ptr<dtk::ListObserver<std::shared_ptr<timeline::Player> > > _playersObserver;
            std::shared_ptr<dtk::ValueObserver<timeline::Compare> > _compareObserver;
        };
    }
}