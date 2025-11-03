// SPDX-License-Identifier: BSD-3-Clause
// Copyright Contributors to the tlRender project.

#pragma once

#include <tlTimeline/Player.h>

#include <ftk/UI/Action.h>

namespace tl
{
    namespace play
    {
        class App;

        //! Compare actions.
        class CompareActions : public std::enable_shared_from_this<CompareActions>
        {
            FTK_NON_COPYABLE(CompareActions);

        protected:
            void _init(
                const std::shared_ptr<ftk::Context>&,
                const std::shared_ptr<App>&);

            CompareActions() = default;

        public:
            ~CompareActions();

            static std::shared_ptr<CompareActions> create(
                const std::shared_ptr<ftk::Context>&,
                const std::shared_ptr<App>&);

            const std::map<std::string, std::shared_ptr<ftk::Action> >& getActions() const;

        private:
            std::map<std::string, std::shared_ptr<ftk::Action> > _actions;
            std::shared_ptr<ftk::ListObserver<std::shared_ptr<timeline::Player> > > _playersObserver;
            std::shared_ptr<ftk::ValueObserver<timeline::Compare> > _compareObserver;
        };
    }
}