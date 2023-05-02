// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2023 Darby Johnston
// All rights reserved.

#pragma once

#include <tlUI/ScrollArea.h>

namespace tl
{
    namespace ui
    {
        //! Scroll widget.
        class ScrollWidget : public IWidget
        {
            TLRENDER_NON_COPYABLE(ScrollWidget);

        protected:
            void _init(
                const std::shared_ptr<system::Context>&,
                ScrollType = ScrollType::Both,
                const std::shared_ptr<IWidget>& parent = nullptr);

            ScrollWidget();

        public:
            ~ScrollWidget() override;

            //! Create a new scroll widget.
            static std::shared_ptr<ScrollWidget> create(
                const std::shared_ptr<system::Context>&,
                ScrollType = ScrollType::Both,
                const std::shared_ptr<IWidget>& parent = nullptr);

            //! Set the widget.
            void setWidget(const std::shared_ptr<IWidget>&);

            void setGeometry(const math::BBox2i&) override;
            void sizeHintEvent(const SizeHintEvent&) override;

        private:
            TLRENDER_PRIVATE();
        };
    }
}
