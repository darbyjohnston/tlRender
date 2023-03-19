// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2023 Darby Johnston
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
                const std::shared_ptr<system::Context>&,
                const std::shared_ptr<IWidget>& parent = nullptr);

            Spacer();

        public:
            ~Spacer() override;

            //! Create a new spacer.
            static std::shared_ptr<Spacer> create(
                const std::shared_ptr<system::Context>&,
                const std::shared_ptr<IWidget>& parent = nullptr);

            //! Set the spacing role.
            void setSpacingRole(SizeRole);

            void sizeEvent(const SizeEvent&) override;

        private:
            TLRENDER_PRIVATE();
        };
    }
}
