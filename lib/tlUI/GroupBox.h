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

            //! Set the font information.
            void setFontInfo(const imaging::FontInfo&);

            void setGeometry(const math::BBox2i&) override;
            void sizeEvent(const SizeEvent&) override;
            void drawEvent(const DrawEvent&) override;

        private:
            TLRENDER_PRIVATE();
        };
    }
}
