// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2023 Darby Johnston
// All rights reserved.

#pragma once

#include <tlUI/IWidget.h>

namespace tl
{
    namespace ui
    {
        //! Stack layout.
        class StackLayout : public IWidget
        {
            TLRENDER_NON_COPYABLE(StackLayout);

        protected:
            void _init(
                const std::shared_ptr<system::Context>&,
                const std::shared_ptr<IWidget>& parent = nullptr);

            StackLayout();

        public:
            ~StackLayout() override;

            //! Create a new stack layout.
            static std::shared_ptr<StackLayout> create(
                const std::shared_ptr<system::Context>&,
                const std::shared_ptr<IWidget>& parent = nullptr);

            void sizeHint(const SizeHintData&) override;
            void setGeometry(const math::BBox2i&) override;
        };
    }
}
