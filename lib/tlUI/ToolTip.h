// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2023 Darby Johnston
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
            TLRENDER_NON_COPYABLE(ToolTip);

        protected:
            void _init(
                const std::string& text,
                const math::Vector2i& pos,
                const std::shared_ptr<EventLoop>&,
                const std::shared_ptr<system::Context>&);

            ToolTip();

        public:
            virtual ~ToolTip();

            //! Create a new tooltip.
            static std::shared_ptr<ToolTip> create(
                const std::string& text,
                const math::Vector2i& pos,
                const std::shared_ptr<EventLoop>&,
                const std::shared_ptr<system::Context>&);

            void close() override;

            void setGeometry(const math::BBox2i&) override;
            void sizeHintEvent(const SizeHintEvent&) override;
            void drawEvent(
                const math::BBox2i&,
                const DrawEvent&) override;

        private:
            TLRENDER_PRIVATE();
        };
    }
}