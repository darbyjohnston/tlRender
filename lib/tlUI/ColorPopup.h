// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2025 Darby Johnston
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
            DTK_NON_COPYABLE(ColorPopup);

        protected:
            void _init(
                const dtk::Color4F&,
                const std::shared_ptr<dtk::Context>&,
                const std::shared_ptr<IWidget>& parent);

            ColorPopup();

        public:
            virtual ~ColorPopup();

            //! Create a new popup.
            static std::shared_ptr<ColorPopup> create(
                const dtk::Color4F&,
                const std::shared_ptr<dtk::Context>&,
                const std::shared_ptr<IWidget>& parent = nullptr);

            //! Set the callback.
            void setCallback(const std::function<void(const dtk::Color4F&)>&);

        private:
            DTK_PRIVATE();
        };
    }
}
