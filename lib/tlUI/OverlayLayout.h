// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2025 Darby Johnston
// All rights reserved.

#pragma once

#include <tlUI/IWidget.h>

namespace tl
{
    namespace ui
    {
        //! Overlay layout.
        class OverlayLayout : public IWidget
        {
            TLRENDER_NON_COPYABLE(OverlayLayout);

        protected:
            void _init(
                const std::shared_ptr<dtk::Context>&,
                const std::shared_ptr<IWidget>& parent);

            OverlayLayout();

        public:
            virtual ~OverlayLayout();

            //! Create a new layout.
            static std::shared_ptr<OverlayLayout> create(
                const std::shared_ptr<dtk::Context>&,
                const std::shared_ptr<IWidget>& parent = nullptr);

            //! Set the margin role.
            void setMarginRole(SizeRole);

            void setGeometry(const dtk::Box2I&) override;
            void sizeHintEvent(const SizeHintEvent&) override;

        private:
            TLRENDER_PRIVATE();
        };
    }
}
