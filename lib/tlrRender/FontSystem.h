// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021 Darby Johnston
// All rights reserved.

#pragma once

#include <tlrAV/Image.h>

#include <tlrCore/BBox.h>
#include <tlrCore/Cache.h>

#include <freetype2/ft2build.h>
#include FT_FREETYPE_H
#include FT_GLYPH_H

#include <codecvt>
#include <locale>
#include <map>

namespace tlr
{
    namespace render
    {
#if defined(_WINDOWS)
        //! \bug https://social.msdn.microsoft.com/Forums/vstudio/en-US/8f40dcd8-c67f-4eba-9134-a19b9178e481/vs-2015-rc-linker-stdcodecvt-error?forum=vcgeneral
        typedef unsigned int tlr_char_t;
#else // DJV_PLATFORM_WINDOWS
        typedef char32_t tlr_char_t;
#endif // DJV_PLATFORM_WINDOWS

        //! Font families.
        enum class FontFamily
        {
            NotoSans,
            NotoMono
        };

        //! Font information.
        struct FontInfo
        {
            FontInfo() noexcept;
            FontInfo(FontFamily, uint16_t size);

            FontFamily family = FontFamily::NotoSans;
            uint16_t size = 0;

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
            std::shared_ptr<imaging::Image> image;
            math::Vector2f offset;
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
            TLR_NON_COPYABLE(FontSystem);

        protected:
            void _init();
            FontSystem();

        public:
            ~FontSystem();

            //! Create a new font system.
            static std::shared_ptr<FontSystem> create();

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
            math::Vector2f measure(const std::string&, const FontInfo&);

            //! Measure the size of glyphs.
            std::vector<math::BBox2f> measureGlyphs(const std::string&, const FontInfo&);

            ///@}

            //! \name Glyphs
            ///@{

            //! Get font glyphs.
            std::vector<std::shared_ptr<Glyph> > getGlyphs(const std::string&, const FontInfo&);

            ///@}

        private:
            std::shared_ptr<Glyph> _getGlyph(uint32_t code, const FontInfo&);
            void _measure(
                const std::basic_string<tlr_char_t>& utf32,
                const FontInfo&,
                uint16_t maxLineWidth,
                math::Vector2f&,
                std::vector<math::BBox2f>* = nullptr);

            FT_Library _ftLibrary = nullptr;
            std::map<FontFamily, FT_Face> _ftFaces;
            std::wstring_convert<std::codecvt_utf8<tlr_char_t>, tlr_char_t> _utf32Convert;
            memory::Cache<GlyphInfo, std::shared_ptr<Glyph> > _glyphCache;
        };
    }
}
