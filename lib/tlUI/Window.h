// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2023 Darby Johnston
// All rights reserved.

#pragma once

#include <tlUI/IWidget.h>

namespace tl
{
    namespace ui
    {
        //! Window.
        class Window : public IWidget
        {
            TLRENDER_NON_COPYABLE(Window);

        protected:
            void _init(
                const std::shared_ptr<system::Context>&,
                const std::shared_ptr<IWidget>& parent = nullptr);

            Window();

        public:
            ~Window() override;

            //! Create a new window.
            static std::shared_ptr<Window> create(
                const std::shared_ptr<system::Context>&,
                const std::shared_ptr<IWidget>& parent = nullptr);

            void sizeHint(const SizeHintData&) override;
            void setGeometry(const math::BBox2i&) override;
            void draw(const DrawData&) override;
        };
    }
}
