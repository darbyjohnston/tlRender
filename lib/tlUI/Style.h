// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2025 Darby Johnston
// All rights reserved.

#pragma once

#include <tlCore/Util.h>

#include <dtk/core/Color.h>
#include <dtk/core/FontSystem.h>
#include <dtk/core/ObservableValue.h>

namespace dtk
{
    class Context;
}

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
            Slider,
            Handle,
            HandleSmall,
            Swatch,
            SwatchLarge,
            Shadow,
            DragLength,

            Count,
            First = None
        };
        DTK_ENUM(SizeRole);

        //! Get the default size roles.
        std::map<SizeRole, int> defaultSizeRoles();

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
            ToolTipWindow,
            ToolTipText,

            InOut,
            FrameMarker,
            VideoCache,
            AudioCache,
            VideoClip,
            VideoGap,
            AudioClip,
            AudioGap,
            Transition,
            
            Red,
            Green,
            Blue,
            Cyan,
            Magenta,
            Yellow,

            Count,
            First = None
        };
        DTK_ENUM(ColorRole);

        //! Get default color roles.
        std::map<ColorRole, dtk::Color4F> defaultColorRoles();

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
        DTK_ENUM(FontRole);

        //! Get default font roles.
        std::map<FontRole, dtk::FontInfo> defaultFontRoles();

        //! Style.
        class Style : public std::enable_shared_from_this<Style>
        {
            TLRENDER_NON_COPYABLE(Style);

        protected:
            void _init(
                const std::shared_ptr<dtk::Context>&);

            Style();

        public:
            ~Style();

            //! Create a new style.
            static std::shared_ptr<Style> create(
                const std::shared_ptr<dtk::Context>&);

            //! Get a size role.
            int getSizeRole(SizeRole, float scale) const;

            //! Set a size role.
            void setSizeRole(SizeRole, int);

            //! Set the size roles.
            void setSizeRoles(const std::map<SizeRole, int>&);

            //! Get a color role.
            dtk::Color4F getColorRole(ColorRole) const;

            //! Set a color role.
            void setColorRole(ColorRole, const dtk::Color4F&);

            //! Set the color roles.
            void setColorRoles(const std::map<ColorRole, dtk::Color4F>&);

            //! Get a font role.
            dtk::FontInfo getFontRole(FontRole, float scale) const;

            //! Set a font role.
            void setFontRole(FontRole, const dtk::FontInfo&);

            //! Set the font roles.
            void setFontRoles(const std::map<FontRole, dtk::FontInfo>&);

            //! Observe style changes.
            std::shared_ptr<dtk::IObservableValue<bool> > observeChanged() const;

        private:
            std::map<SizeRole, int> _sizeRoles;
            std::map<ColorRole, dtk::Color4F> _colorRoles;
            std::map<FontRole, dtk::FontInfo> _fontRoles;

            TLRENDER_PRIVATE();
        };

        //! \name Serialize
        ///@{

        void to_json(nlohmann::json&, const std::map<ColorRole, dtk::Color4F>&);

        void from_json(const nlohmann::json&, std::map<ColorRole, dtk::Color4F>&);

        ///@}
    }
}

#include <tlUI/StyleInline.h>
