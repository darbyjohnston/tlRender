// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2023 Darby Johnston
// All rights reserved.

#pragma once

#include <tlUI/IWidget.h>

namespace tl
{
    namespace ui
    {
        //! Group box.
        class GroupBox : public IWidget
        {
            TLRENDER_NON_COPYABLE(GroupBox);

        protected:
            void _init(
                const std::shared_ptr<system::Context>&,
                const std::shared_ptr<IWidget>& parent = nullptr);

            GroupBox();

        public:
            ~GroupBox() override;

            //! Create a new group box.
            static std::shared_ptr<GroupBox> create(
                const std::shared_ptr<system::Context>&,
                const std::shared_ptr<IWidget>& parent = nullptr);

            //! Set the text.
            void setText(const std::string&);

            //! Set the font role.
            void setFontRole(FontRole);

            void setGeometry(const math::BBox2i&) override;
            void sizeHintEvent(const SizeHintEvent&) override;
            void clipEvent(
                const math::BBox2i&,
                bool,
                const ClipEvent&) override;
            void drawEvent(
                const math::BBox2i&,
                const DrawEvent&) override;

        private:
            TLRENDER_PRIVATE();
        };
    }
}
