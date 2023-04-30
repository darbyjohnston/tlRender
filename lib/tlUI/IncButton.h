// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2023 Darby Johnston
// All rights reserved.

#pragma once

#include <tlUI/IButton.h>

namespace tl
{
    namespace ui
    {
        //! Numeric widget increment button.
        class IncButton : public IButton
        {
            TLRENDER_NON_COPYABLE(IncButton);

        protected:
            void _init(
                const std::shared_ptr<system::Context>&,
                const std::shared_ptr<IWidget>& parent = nullptr);

            IncButton();

        public:
            ~IncButton() override;

            //! Create a new increment button.
            static std::shared_ptr<IncButton> create(
                const std::shared_ptr<system::Context>&,
                const std::shared_ptr<IWidget>& parent = nullptr);

            void sizeHintEvent(const SizeHintEvent&) override;
            void drawEvent(const DrawEvent&) override;

        private:
            TLRENDER_PRIVATE();
        };
    }
}
