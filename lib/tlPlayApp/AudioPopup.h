// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2025 Darby Johnston
// All rights reserved.

#pragma once

#include <tlUI/IWidgetPopup.h>

namespace tl
{
    namespace play_app
    {
        class App;

        //! Audio popup.
        class AudioPopup : public ui::IWidgetPopup
        {
            DTK_NON_COPYABLE(AudioPopup);

        protected:
            void _init(
                const std::shared_ptr<dtk::Context>&,
                const std::shared_ptr<App>&,
                const std::shared_ptr<IWidget>& parent);

            AudioPopup();

        public:
            virtual ~AudioPopup();

            static std::shared_ptr<AudioPopup> create(
                const std::shared_ptr<dtk::Context>&,
                const std::shared_ptr<App>&,
                const std::shared_ptr<IWidget>& parent = nullptr);

        private:
            void _widgetUpdate();

            DTK_PRIVATE();
        };
    }
}
