// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2023 Darby Johnston
// All rights reserved.

#pragma once

#include <tlUI/IMenuPopup.h>

namespace tl
{
    namespace play_gl
    {
        class App;

        //! Audio popup.
        class AudioPopup : public ui::IMenuPopup
        {
            TLRENDER_NON_COPYABLE(AudioPopup);

        protected:
            void _init(
                const std::shared_ptr<App>&,
                const std::shared_ptr<system::Context>&,
                const std::shared_ptr<IWidget>& parent);

            AudioPopup();

        public:
            virtual ~AudioPopup();

            static std::shared_ptr<AudioPopup> create(
                const std::shared_ptr<App>&,
                const std::shared_ptr<system::Context>&,
                const std::shared_ptr<IWidget>& parent = nullptr);

        private:
            TLRENDER_PRIVATE();
        };
    }
}
