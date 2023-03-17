// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2023 Darby Johnston
// All rights reserved.

#pragma once

#include <tlUI/IWidget.h>

namespace tl
{
    namespace ui
    {
        //! Orientation.
        enum class Orientation
        {
            Horizontal,
            Vertical
        };

        //! Row layout.
        class RowLayout : public IWidget
        {
            TLRENDER_NON_COPYABLE(RowLayout);

        protected:
            void _init(
                Orientation,
                const std::shared_ptr<system::Context>&,
                const std::shared_ptr<IWidget>& parent = nullptr);

            RowLayout();

        public:
            ~RowLayout() override;

            //! Create a new row layout.
            static std::shared_ptr<RowLayout> create(
                Orientation,
                const std::shared_ptr<system::Context>&,
                const std::shared_ptr<IWidget>& parent = nullptr);

            void sizeHint(const SizeHintData&) override;
            void setGeometry(const math::BBox2i&) override;

        private:
            Orientation _orientation;
        };
    }
}
