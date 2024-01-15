// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2024 Darby Johnston
// All rights reserved.

#pragma once

#include <tlUI/IWidgetPopup.h>

namespace tl
{
    namespace ui
    {
        //! Color popup.
        class ColorPopup : public IWidgetPopup
        {
            TLRENDER_NON_COPYABLE(ColorPopup);

        protected:
            void _init(
                const image::Color4f&,
                const std::shared_ptr<system::Context>&,
                const std::shared_ptr<IWidget>& parent);

            ColorPopup();

        public:
            virtual ~ColorPopup();

            //! Create a new popup.
            static std::shared_ptr<ColorPopup> create(
                const image::Color4f&,
                const std::shared_ptr<system::Context>&,
                const std::shared_ptr<IWidget>& parent = nullptr);

            //! Set the callback.
            void setCallback(const std::function<void(const image::Color4f&)>&);

        private:
            TLRENDER_PRIVATE();
        };
    }
}
