// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2025 Darby Johnston
// All rights reserved.

#pragma once

#include <tlUI/IPopup.h>

namespace tl
{
    namespace ui
    {
        //! Tool tip.
        class ToolTip : public IPopup
        {
            DTK_NON_COPYABLE(ToolTip);

        protected:
            void _init(
                const std::string& text,
                const dtk::V2I& pos,
                const std::shared_ptr<IWidget>&,
                const std::shared_ptr<dtk::Context>&);

            ToolTip();

        public:
            virtual ~ToolTip();

            //! Create a new tooltip.
            static std::shared_ptr<ToolTip> create(
                const std::string& text,
                const dtk::V2I& pos,
                const std::shared_ptr<IWidget>&,
                const std::shared_ptr<dtk::Context>&);

            void close() override;

            void setGeometry(const dtk::Box2I&) override;
            void sizeHintEvent(const SizeHintEvent&) override;
            void drawEvent(const dtk::Box2I&, const DrawEvent&) override;

        private:
            DTK_PRIVATE();
        };
    }
}
