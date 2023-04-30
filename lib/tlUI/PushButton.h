// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2023 Darby Johnston
// All rights reserved.

#pragma once

#include <tlUI/IButton.h>

namespace tl
{
    namespace ui
    {
        //! Push button.
        class PushButton : public IButton
        {
            TLRENDER_NON_COPYABLE(PushButton);

        protected:
            void _init(
                const std::shared_ptr<system::Context>&,
                const std::shared_ptr<IWidget>& parent = nullptr);

            PushButton();

        public:
            ~PushButton() override;

            //! Create a new push button.
            static std::shared_ptr<PushButton> create(
                const std::shared_ptr<system::Context>&,
                const std::shared_ptr<IWidget>& parent = nullptr);

            void setText(const std::string&) override;
            void setFontRole(FontRole) override;

            bool acceptsKeyFocus() const override;
            void sizeHintEvent(const SizeHintEvent&) override;
            void clipEvent(bool, const ClipEvent&) override;
            void drawEvent(const DrawEvent&) override;
            void keyPressEvent(KeyEvent&) override;
            void keyReleaseEvent(KeyEvent&) override;

        private:
            TLRENDER_PRIVATE();
        };
    }
}
