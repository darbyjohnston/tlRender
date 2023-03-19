// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2023 Darby Johnston
// All rights reserved.

#pragma once

#include <tlCore/Color.h>
#include <tlCore/Context.h>

namespace tl
{
    namespace ui
    {
        //! Size roles.
        enum class SizeRole
        {
            None,
            Margin,
            MarginSmall,
            Spacing,
            SpacingSmall,
            Border,
            ScrollArea
        };

        //! Color roles.
        enum class ColorRole
        {
            None,
            Window,
            Base,
            Button,
            Text,
            Hover,
            Pressed,
            Checked,
            Red,
            Green,
            Blue,
            Cyan,
            Magenta,
            Yellow
        };

        //! Style.
        class Style : public std::enable_shared_from_this<Style>
        {
            TLRENDER_NON_COPYABLE(Style);

        protected:
            void _init(
                const std::shared_ptr<system::Context>&);

            Style();

        public:
            ~Style();

            //! Create a new style.
            static std::shared_ptr<Style> create(
                const std::shared_ptr<system::Context>&);

            //! Get a size role.
            int getSizeRole(SizeRole) const;

            //! Get a color role.
            imaging::Color4f getColorRole(ColorRole) const;

        private:
            TLRENDER_PRIVATE();
        };
    }
}
