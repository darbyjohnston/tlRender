// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2025 Darby Johnston
// All rights reserved.

#pragma once

#include <dtk/ui/IMenuPopup.h>

namespace tl
{
    namespace play_app
    {
        //! Speed popup.
        class SpeedPopup : public dtk::IMenuPopup
        {
            DTK_NON_COPYABLE(SpeedPopup);

        protected:
            void _init(
                const std::shared_ptr<dtk::Context>&,
                double defaultSpeed,
                const std::shared_ptr<IWidget>& parent);

            SpeedPopup();

        public:
            virtual ~SpeedPopup();

            static std::shared_ptr<SpeedPopup> create(
                const std::shared_ptr<dtk::Context>&,
                double defaultSpeed,
                const std::shared_ptr<IWidget>& parent = nullptr);

            void setCallback(const std::function<void(double)>&);

        private:
            void _menuUpdate();

            DTK_PRIVATE();
        };
    }
}
