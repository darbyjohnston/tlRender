// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2024 Darby Johnston
// All rights reserved.

#pragma once

#include <tlTimeline/Player.h>

#include <feather-tk/ui/Action.h>

namespace tl
{
    namespace play
    {
        class App;

        //! File actions.
        class FileActions : public std::enable_shared_from_this<FileActions>
        {
            FEATHER_TK_NON_COPYABLE(FileActions);

        protected:
            void _init(
                const std::shared_ptr<feather_tk::Context>&,
                const std::shared_ptr<App>&);

            FileActions() = default;

        public:
            ~FileActions();

            static std::shared_ptr<FileActions> create(
                const std::shared_ptr<feather_tk::Context>&,
                const std::shared_ptr<App>&);

            const std::map<std::string, std::shared_ptr<feather_tk::Action> >& getActions() const;

        private:
            std::map<std::string, std::shared_ptr<feather_tk::Action> > _actions;
            std::shared_ptr<feather_tk::ListObserver<std::shared_ptr<timeline::Player> > > _playersObserver;
            std::shared_ptr<feather_tk::ValueObserver<std::shared_ptr<timeline::Player> > > _playerObserver;
        };
    }
}