// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2023 Darby Johnston
// All rights reserved.

#pragma once

#include <tlCore/BBox.h>
#include <tlCore/Util.h>

#include <memory>
#include <string>
#include <vector>

namespace tl
{
    namespace system
    {
        class Context;
    }

    namespace imaging
    {
#if defined(_WINDOWS)
        //! \bug https://social.msdn.microsoft.com/Forums/vstudio/en-US/8f40dcd8-c67f-4eba-9134-a19b9178e481/vs-2015-rc-linker-stdcodecvt-error?forum=vcgeneral
        typedef unsigned int tl_char_t;
#else // _WINDOWS
        typedef char32_t tl_char_t;
#endif // _WINDOWS

        //! Get font data.
        std::vector<uint8_t> getFontData(const std::string&);

        //! Font information.
        struct FontInfo
        {
            FontInfo() noexcept;
            FontInfo(const std::string& family, uint16_t size);

            std::string family = "NotoSans-Regular";
            uint16_t size = 12;

            bool operator == (const FontInfo&) const noexcept;
            bool operator < (const FontInfo&) const;
        };

        //! Font metrics.
        struct FontMetrics
        {
            int16_t ascender = 0;
            int16_t descender = 0;
            int16_t lineHeight = 0;
        };

        //! Font glyph information.
        struct GlyphInfo
        {
            GlyphInfo() noexcept;
            GlyphInfo(uint32_t code, const FontInfo&) noexcept;

            uint32_t code = 0;
            FontInfo fontInfo;

            bool operator == (const GlyphInfo&) const noexcept;
            bool operator < (const GlyphInfo&) const;
        };

        //! Font glyph.
        struct Glyph
        {
            GlyphInfo glyphInfo;
            std::vector<uint8_t> data;
            uint16_t width = 0;
            uint16_t height = 0;
            math::Vector2i offset;
            int16_t advance = 0;
            int32_t lsbDelta = 0;
            int32_t rsbDelta = 0;
        };

        //! Font system.
        //!
        //! \todo Add support for gamma correction?
        //! - https://www.freetype.org/freetype2/docs/text-rendering-general.html
        class FontSystem : public std::enable_shared_from_this<FontSystem>
        {
            TLRENDER_NON_COPYABLE(FontSystem);

        protected:
            void _init(const std::shared_ptr<system::Context>&);
            FontSystem();

        public:
            ~FontSystem();

            //! Create a new font system.
            static std::shared_ptr<FontSystem> create(const std::shared_ptr<system::Context>&);

            //! \name Information
            ///@{

            //! Get the glyph cache size.
            size_t getGlyphCacheSize() const;

            //! Get the percentage of the glyph cache in use.
            float getGlyphCachePercentage() const;

            ///@}

            //! \name Measure
            ///@{

            //! Get font metrics.
            FontMetrics getMetrics(const FontInfo&);

            //! Measure the size of text.
            math::Vector2i measure(
                const std::string&,
                const FontInfo&,
                uint16_t maxLineWidth = 0);

            //! Measure the size of glyphs.
            std::vector<math::BBox2i> measureGlyphs(
                const std::string&,
                const FontInfo&,
                uint16_t maxLineWidth = 0);

            ///@}

            //! \name Glyphs
            ///@{

            //! Get font glyphs.
            std::vector<std::shared_ptr<Glyph> > getGlyphs(
                const std::string&,
                const FontInfo&);

            ///@}

        private:
            TLRENDER_PRIVATE();
        };
    }
}
