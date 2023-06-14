// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2023 Darby Johnston
// All rights reserved.

#pragma once

#include <tlUI/IWidget.h>

namespace tl
{
    namespace ui
    {
        //! Divider.
        class Divider : public IWidget
        {
            TLRENDER_NON_COPYABLE(Divider);

        protected:
            void _init(
                Orientation,
                const std::shared_ptr<system::Context>&,
                const std::shared_ptr<IWidget>& parent = nullptr);

            Divider();

        public:
            ~Divider() override;

            //! Create a new widget.
            static std::shared_ptr<Divider> create(
                Orientation,
                const std::shared_ptr<system::Context>&,
                const std::shared_ptr<IWidget>& parent = nullptr);

            void sizeHintEvent(const SizeHintEvent&) override;
        };
    }
}
