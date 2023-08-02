// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2023 Darby Johnston
// All rights reserved.

#pragma once

#include <tlUI/Action.h>

namespace tl
{
    namespace play_gl
    {
        class App;
        class MainWindow;

        //! Playback actions.
        class PlaybackActions : public std::enable_shared_from_this<PlaybackActions>
        {
            TLRENDER_NON_COPYABLE(PlaybackActions);

        protected:
            void _init(
                const std::shared_ptr<MainWindow>&,
                const std::shared_ptr<App>&,
                const std::shared_ptr<system::Context>&);

            PlaybackActions();

        public:
            ~PlaybackActions();

            static std::shared_ptr<PlaybackActions> create(
                const std::shared_ptr<MainWindow>&,
                const std::shared_ptr<App>&,
                const std::shared_ptr<system::Context>&);

            const std::map<std::string, std::shared_ptr<ui::Action> >& getActions() const;

        private:
            TLRENDER_PRIVATE();
        };
    }
}
