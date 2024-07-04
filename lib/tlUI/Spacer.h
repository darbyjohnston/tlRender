// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2024 Darby Johnston
// All rights reserved.

#pragma once

#include <tlUI/IWidget.h>

namespace tl
{
    namespace ui
    {
        //! Spacer.
        class Spacer : public IWidget
        {
            TLRENDER_NON_COPYABLE(Spacer);

        protected:
            void _init(
                Orientation,
                const std::shared_ptr<system::Context>&,
                const std::shared_ptr<IWidget>& parent);

            Spacer();

        public:
            virtual ~Spacer();

            //! Create a new widget.
            static std::shared_ptr<Spacer> create(
                Orientation,
                const std::shared_ptr<system::Context>&,
                const std::shared_ptr<IWidget>& parent = nullptr);

            //! Set the size role.
            void setSizeRole(SizeRole);

            void sizeHintEvent(const SizeHintEvent&) override;

        private:
            TLRENDER_PRIVATE();
        };
    }
}
