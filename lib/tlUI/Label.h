// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2023 Darby Johnston
// All rights reserved.

#pragma once

#include <tlUI/IWidget.h>

namespace tl
{
    namespace ui
    {
        //! Text label.
        class Label : public IWidget
        {
            TLRENDER_NON_COPYABLE(Label);

        protected:
            void _init(
                const std::shared_ptr<system::Context>&,
                const std::shared_ptr<IWidget>& parent = nullptr);

            Label();

        public:
            ~Label() override;

            //! Create a new widget.
            static std::shared_ptr<Label> create(
                const std::shared_ptr<system::Context>&,
                const std::shared_ptr<IWidget>& parent = nullptr);

            //! Set the text.
            void setText(const std::string&);

            //! Set the number of characters to show. A value of
            //! zero (the default) shows the entire text.
            void setTextWidth(size_t);

            //! Set the margin role.
            void setMarginRole(SizeRole);

            //! Set the font role.
            void setFontRole(FontRole);

            void sizeHintEvent(const SizeHintEvent&) override;
            void clipEvent(
                const math::BBox2i&,
                bool,
                const ClipEvent&) override;
            void drawEvent(
                const math::BBox2i&,
                const DrawEvent&) override;

        private:
            std::string _getText() const;
            TLRENDER_PRIVATE();
        };
    }
}
