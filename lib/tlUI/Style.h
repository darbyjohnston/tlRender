// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2023 Darby Johnston
// All rights reserved.

#pragma once

#include <tlCore/Color.h>
#include <tlCore/Context.h>
#include <tlCore/FontSystem.h>
#include <tlCore/ValueObserver.h>

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
            MarginLarge,
            MarginInside,
            MarginDialog,
            Spacing,
            SpacingSmall,
            SpacingLarge,
            SpacingTool,
            Border,
            ScrollArea,
            Handle,
            HandleSmall,
            Shadow,
            DragLength,

            Count,
            First = None
        };
        TLRENDER_ENUM(SizeRole);
        TLRENDER_ENUM_SERIALIZE(SizeRole);

        //! Color roles.
        enum class ColorRole
        {
            None,
            Window,
            Base,
            Button,
            Text,
            TextDisabled,
            Border,
            Hover,
            Pressed,
            Checked,
            KeyFocus,
            Overlay,
            Red,
            Green,
            Blue,
            Cyan,
            Magenta,
            Yellow,

            Count,
            First = None
        };
        TLRENDER_ENUM(ColorRole);
        TLRENDER_ENUM_SERIALIZE(ColorRole);

        //! Font roles.
        enum class FontRole
        {
            None,
            Label,
            Mono,
            Title,

            Count,
            First = None
        };
        TLRENDER_ENUM(FontRole);
        TLRENDER_ENUM_SERIALIZE(FontRole);

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
            int getSizeRole(SizeRole, float scale) const;

            //! Set a size role.
            void setSizeRole(SizeRole, int);

            //! Get a color role.
            imaging::Color4f getColorRole(ColorRole) const;

            //! Set a color role.
            void setColorRole(ColorRole, const imaging::Color4f&);

            //! Get a font role.
            imaging::FontInfo getFontRole(FontRole, float scale) const;

            //! Set a font role.
            void setFontRole(FontRole, const imaging::FontInfo&);

            //! Observe style changes.
            std::shared_ptr<observer::IValue<bool> > observeChanged() const;

        private:
            std::vector<int> _sizeRoles;
            std::vector<imaging::Color4f> _colorRoles;
            std::vector<imaging::FontInfo> _fontRoles;

            TLRENDER_PRIVATE();
        };
    }
}

#include <tlUI/StyleInline.h>
