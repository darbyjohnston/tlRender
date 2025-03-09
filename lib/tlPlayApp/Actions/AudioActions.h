// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2025 Darby Johnston
// All rights reserved.

#pragma once

#include <tlPlayApp/Actions/IActions.h>

namespace tl
{
    namespace play
    {
        //! Audio actions.
        class AudioActions : public IActions
        {
            DTK_NON_COPYABLE(AudioActions);

        protected:
            void _init(
                const std::shared_ptr<dtk::Context>&,
                const std::shared_ptr<App>&);

            AudioActions() = default;

        public:
            ~AudioActions();

            static std::shared_ptr<AudioActions> create(
                const std::shared_ptr<dtk::Context>&,
                const std::shared_ptr<App>&);
        };
    }
}
