// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2023 Darby Johnston
// All rights reserved.

#pragma once

#include <tlUI/IWidget.h>

namespace tl
{
    namespace ui
    {
        //! Row layout.
        class RowLayout : public IWidget
        {
            TLRENDER_NON_COPYABLE(RowLayout);

        protected:
            void _init(
                Orientation,
                const std::string& name,
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

            //! Set the margin role.
            void setMarginRole(SizeRole);

            //! Set the spacing role.
            void setSpacingRole(SizeRole);

            void setGeometry(const math::BBox2i&) override;
            void sizeHintEvent(const SizeHintEvent&) override;

        private:
            TLRENDER_PRIVATE();
        };

        //! Horizontal layout.
        class HorizontalLayout : public RowLayout
        {
            TLRENDER_NON_COPYABLE(HorizontalLayout);

        protected:
            void _init(
                const std::shared_ptr<system::Context>&,
                const std::shared_ptr<IWidget>& parent = nullptr);

            HorizontalLayout();

        public:
            ~HorizontalLayout() override;

            //! Create a new horizontal layout.
            static std::shared_ptr<HorizontalLayout> create(
                const std::shared_ptr<system::Context>&,
                const std::shared_ptr<IWidget>& parent = nullptr);
        };

        //! Vertical layout.
        class VerticalLayout : public RowLayout
        {
            TLRENDER_NON_COPYABLE(VerticalLayout);

        protected:
            void _init(
                const std::shared_ptr<system::Context>&,
                const std::shared_ptr<IWidget>& parent = nullptr);

            VerticalLayout();

        public:
            ~VerticalLayout() override;

            //! Create a new vertical layout.
            static std::shared_ptr<VerticalLayout> create(
                const std::shared_ptr<system::Context>&,
                const std::shared_ptr<IWidget>& parent = nullptr);
        };
    }
}
