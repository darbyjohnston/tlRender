// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2025 Darby Johnston
// All rights reserved.

#pragma once

#include <tlPlayApp/Actions/IActions.h>

namespace tl
{
    namespace play
    {
        //! Playback actions.
        class PlaybackActions : public IActions
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

        private:
            void _setPlayer(const std::shared_ptr<timeline::Player>&);
            void _playbackUpdate();
            void _loopUpdate();

            timeline::Playback _playbackPrev = timeline::Playback::Forward;

            DTK_PRIVATE();
        };
    }
}
