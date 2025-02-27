// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2025 Darby Johnston
// All rights reserved.

#pragma once

#include <dtk/ui/Action.h>

namespace tl
{
    namespace play
    {
        class App;

        //! Playback actions.
        class PlaybackActions : public std::enable_shared_from_this<PlaybackActions>
        {
            DTK_NON_COPYABLE(PlaybackActions);

        protected:
            void _init(
                const std::shared_ptr<dtk::Context>&,
                const std::shared_ptr<App>&);

            PlaybackActions();

        public:
            ~PlaybackActions();

            static std::shared_ptr<PlaybackActions> create(
                const std::shared_ptr<dtk::Context>&,
                const std::shared_ptr<App>&);

            const std::map<std::string, std::shared_ptr<dtk::Action> >& getActions() const;

        private:
            DTK_PRIVATE();
        };
    }
}
