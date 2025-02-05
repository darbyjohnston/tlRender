// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2025 Darby Johnston
// All rights reserved.

#pragma once

#include <tlUI/IWidget.h>

namespace tl
{
    namespace ui
    {
        //! Divider widget.
        class Divider : public IWidget
        {
            DTK_NON_COPYABLE(Divider);

        protected:
            void _init(
                Orientation,
                const std::shared_ptr<dtk::Context>&,
                const std::shared_ptr<IWidget>& parent);

            Divider();

        public:
            virtual ~Divider();

            //! Create a new widget.
            static std::shared_ptr<Divider> create(
                Orientation,
                const std::shared_ptr<dtk::Context>&,
                const std::shared_ptr<IWidget>& parent = nullptr);

            void sizeHintEvent(const SizeHintEvent&) override;

        private:
            DTK_PRIVATE();
        };
    }
}
