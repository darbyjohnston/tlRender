// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2023 Darby Johnston
// All rights reserved.

#pragma once

#include <tlUI/IButton.h>

namespace tl
{
    namespace ui
    {
        //! List button.
        class ListButton : public IButton
        {
            TLRENDER_NON_COPYABLE(ListButton);

        protected:
            void _init(
                const std::shared_ptr<system::Context>&,
                const std::shared_ptr<IWidget>& parent = nullptr);

            ListButton();

        public:
            ~ListButton() override;

            //! Create a new widget.
            static std::shared_ptr<ListButton> create(
                const std::shared_ptr<system::Context>&,
                const std::shared_ptr<IWidget>& parent = nullptr);

            void setText(const std::string&) override;
            void setFontRole(FontRole) override;

            bool acceptsKeyFocus() const override;
            void sizeHintEvent(const SizeHintEvent&) override;
            void clipEvent(
                const math::BBox2i&,
                bool,
                const ClipEvent&) override;
            void drawEvent(
                const math::BBox2i&,
                const DrawEvent&) override;
            void keyPressEvent(KeyEvent&) override;
            void keyReleaseEvent(KeyEvent&) override;

        private:
            TLRENDER_PRIVATE();
        };
    }
}