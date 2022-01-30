// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2022 Darby Johnston
// All rights reserved.

#include <tlrCore/FontSystem.h>

#include <tlrCore/LRUCache.h>

#include <Fonts/NotoMono-Regular.font>
#include <Fonts/NotoSans-Regular.font>

#include <ft2build.h>
#include FT_FREETYPE_H
#include FT_GLYPH_H

#include <codecvt>
#include <locale>
#include <map>

namespace tlr
{
    namespace imaging
    {
        FontInfo::FontInfo() noexcept
        {}

        FontInfo::FontInfo(FontFamily family, uint16_t size) :
            family(family),
            size(size)
        {}

        bool FontInfo::operator == (const FontInfo & other) const noexcept
        {
            return family == other.family && size == other.size;
        }

        bool FontInfo::operator < (const FontInfo& other) const
        {
            return std::tie(family, size) < std::tie(other.family, other.size);
        }

        GlyphInfo::GlyphInfo() noexcept
        {}

        GlyphInfo::GlyphInfo(uint32_t code, const FontInfo& fontInfo) noexcept :
            code(code),
            fontInfo(fontInfo)
        {}

        bool GlyphInfo::operator < (const GlyphInfo& other) const
        {
            return std::tie(code, fontInfo) < std::tie(other.code, other.fontInfo);
        }

        struct FontSystem::Private
        {
            std::shared_ptr<Glyph> getGlyph(uint32_t code, const FontInfo&);
            void measure(
                const std::basic_string<tlr_char_t>& utf32,
                const FontInfo&,
                uint16_t maxLineWidth,
                math::Vector2i&,
                std::vector<math::BBox2i>* = nullptr);

            FT_Library ftLibrary = nullptr;
            std::map<FontFamily, FT_Face> ftFaces;
            std::wstring_convert<std::codecvt_utf8<tlr_char_t>, tlr_char_t> utf32Convert;
            memory::LRUCache<GlyphInfo, std::shared_ptr<Glyph> > glyphCache;
        };

        void FontSystem::_init()
        {
            TLR_PRIVATE_P();

            FT_Error ftError = FT_Init_FreeType(&p.ftLibrary);
            if (ftError)
            {
                throw std::runtime_error("FreeType cannot be initialized");
            }

            ftError = FT_New_Memory_Face(p.ftLibrary, NotoSans_Regular_ttf, NotoSans_Regular_ttf_len, 0, &p.ftFaces[FontFamily::NotoSans]);
            if (ftError)
            {
                throw std::runtime_error("Cannot create font");
            }
            ftError = FT_New_Memory_Face(p.ftLibrary, NotoMono_Regular_ttf, NotoMono_Regular_ttf_len, 0, &p.ftFaces[FontFamily::NotoMono]);
            if (ftError)
            {
                throw std::runtime_error("Cannot create font");
            }
        }

        FontSystem::FontSystem() :
            _p(new Private)
        {}

        FontSystem::~FontSystem()
        {
            TLR_PRIVATE_P();
            if (p.ftLibrary)
            {
                for (const auto& i : p.ftFaces)
                {
                    FT_Done_Face(i.second);
                }
                FT_Done_FreeType(p.ftLibrary);
            }
        }

        std::shared_ptr<FontSystem> FontSystem::create()
        {
            auto out = std::shared_ptr<FontSystem>(new FontSystem);
            out->_init();
            return out;
        }

        size_t FontSystem::getGlyphCacheSize() const
        {
            return _p->glyphCache.getSize();
        }

        float FontSystem::getGlyphCachePercentage() const
        {
            return _p->glyphCache.getPercentageUsed();
        }

        FontMetrics FontSystem::getMetrics(const FontInfo& info)
        {
            TLR_PRIVATE_P();
            FontMetrics out;
            const auto i = p.ftFaces.find(info.family);
            if (i != p.ftFaces.end())
            {
                FT_Error ftError = FT_Set_Pixel_Sizes(i->second, 0, info.size);
                if (ftError)
                {
                    throw std::runtime_error("Cannot set pixel sizes");
                }
                out.ascender = i->second->size->metrics.ascender / 64;
                out.descender = i->second->size->metrics.descender / 64;
                out.lineHeight = i->second->size->metrics.height / 64;
            }
            return out;
        }

        math::Vector2i FontSystem::measure(const std::string& text, const FontInfo& fontInfo)
        {
            TLR_PRIVATE_P();
            math::Vector2i out;
            const auto utf32 = p.utf32Convert.from_bytes(text);
            p.measure(utf32, fontInfo, std::numeric_limits<int16_t>::max(), out);
            return out;
        }

        std::vector<math::BBox2i> FontSystem::measureGlyphs(const std::string& text, const FontInfo& fontInfo)
        {
            TLR_PRIVATE_P();
            std::vector<math::BBox2i> out;
            const auto utf32 = p.utf32Convert.from_bytes(text);
            math::Vector2i size;
            p.measure(utf32, fontInfo, std::numeric_limits<int16_t>::max(), size, &out);
            return out;
        }

        std::vector<std::shared_ptr<Glyph> > FontSystem::getGlyphs(const std::string& text, const FontInfo& fontInfo)
        {
            TLR_PRIVATE_P();
            std::vector<std::shared_ptr<Glyph> > out;
            const auto utf32 = p.utf32Convert.from_bytes(text);
            for (const auto& i : utf32)
            {
                out.push_back(p.getGlyph(i, fontInfo));
            }
            return out;
        }

        std::shared_ptr<Glyph> FontSystem::Private::getGlyph(uint32_t code, const FontInfo& fontInfo)
        {
            std::shared_ptr<Glyph> out;
            if (!glyphCache.get(GlyphInfo(code, fontInfo), out))
            {
                const auto i = ftFaces.find(fontInfo.family);
                if (i != ftFaces.end())
                {
                    if (auto ftGlyphIndex = FT_Get_Char_Index(i->second, code))
                    {
                        FT_Error ftError = FT_Set_Pixel_Sizes(
                            i->second,
                            0,
                            static_cast<int>(fontInfo.size));
                        if (ftError)
                        {
                            throw std::runtime_error("Cannot set pixel sizes");
                        }

                        ftError = FT_Load_Glyph(i->second, ftGlyphIndex, FT_LOAD_FORCE_AUTOHINT);
                        if (ftError)
                        {
                            throw std::runtime_error("Cannot load glyph");
                        }
                        FT_Render_Mode renderMode = FT_RENDER_MODE_NORMAL;
                        uint8_t renderModeChannels = 1;
                        ftError = FT_Render_Glyph(i->second->glyph, renderMode);
                        if (ftError)
                        {
                            throw std::runtime_error("Cannot render glyph");
                        }
                        FT_Glyph ftGlyph;
                        ftError = FT_Get_Glyph(i->second->glyph, &ftGlyph);
                        if (ftError)
                        {
                            throw std::runtime_error("Cannot get glyph");
                        }
                        FT_Vector v;
                        v.x = 0;
                        v.y = 0;
                        ftError = FT_Glyph_To_Bitmap(&ftGlyph, renderMode, &v, 0);
                        if (ftError)
                        {
                            FT_Done_Glyph(ftGlyph);
                            throw std::runtime_error("Cannot convert glyph to a bitmap");
                        }

                        out = std::make_shared<Glyph>();
                        out->glyphInfo = GlyphInfo(code, fontInfo);
                        auto ftBitmapGlyph = reinterpret_cast<FT_BitmapGlyph>(ftGlyph)->bitmap;
                        out->data.resize(ftBitmapGlyph.width * ftBitmapGlyph.rows);
                        out->width = ftBitmapGlyph.width;
                        out->height = ftBitmapGlyph.rows;
                        for (size_t y = 0; y < ftBitmapGlyph.rows; ++y)
                        {
                            uint8_t* dataP = out->data.data() + ftBitmapGlyph.width * y;
                            unsigned char* bitmapP = ftBitmapGlyph.buffer + y * ftBitmapGlyph.pitch;
                            for (size_t x = 0; x < ftBitmapGlyph.width; ++x)
                            {
                                dataP[x] = bitmapP[x];
                            }
                        }
                        out->offset = math::Vector2i(i->second->glyph->bitmap_left, i->second->glyph->bitmap_top);
                        out->advance = i->second->glyph->advance.x / 64;
                        out->lsbDelta = i->second->glyph->lsb_delta;
                        out->rsbDelta = i->second->glyph->rsb_delta;
                        FT_Done_Glyph(ftGlyph);

                        glyphCache.add(out->glyphInfo, out);
                    }
                }
            }
            return out;
        }

        namespace
        {
            constexpr bool isSpace(tlr_char_t c)
            {
                return ' ' == c || '\t' == c;
            }

            constexpr bool isNewline(tlr_char_t c)
            {
                return '\n' == c || '\r' == c;
            }
        }

        void FontSystem::Private::measure(
            const std::basic_string<tlr_char_t>& utf32,
            const FontInfo& fontInfo,
            uint16_t maxLineWidth,
            math::Vector2i& size,
            std::vector<math::BBox2i>* glyphGeom)
        {
            const auto i = ftFaces.find(fontInfo.family);
            if (i != ftFaces.end())
            {
                math::Vector2i pos;
                FT_Error ftError = FT_Set_Pixel_Sizes(
                    i->second,
                    0,
                    static_cast<int>(fontInfo.size));
                if (ftError)
                {
                    throw std::runtime_error("Cannot set pixel sizes");
                }

                pos.y = i->second->size->metrics.height / 64;
                auto textLine = utf32.end();
                int textLineX = 0;
                int32_t rsbDeltaPrev = 0;
                for (auto j = utf32.begin(); j != utf32.end(); ++j)
                {
                    const auto glyph = getGlyph(*j, fontInfo);
                    if (glyph && glyphGeom)
                    {
                        glyphGeom->push_back(math::BBox2i(
                            pos.x,
                            glyph->advance,
                            glyph->advance,
                            i->second->size->metrics.height / 64));
                    }

                    int32_t x = 0;
                    math::Vector2i posAndSize;
                    if (glyph)
                    {
                        x = glyph->advance;
                        if (rsbDeltaPrev - glyph->lsbDelta > 32)
                        {
                            x -= 1;
                        }
                        else if (rsbDeltaPrev - glyph->lsbDelta < -31)
                        {
                            x += 1;
                        }
                        rsbDeltaPrev = glyph->rsbDelta;
                    }
                    else
                    {
                        rsbDeltaPrev = 0;
                    }

                    if (isNewline(*j))
                    {
                        size.x = std::max(size.x, pos.x);
                        pos.x = 0;
                        pos.y += i->second->size->metrics.height / 64;
                        rsbDeltaPrev = 0;
                    }
                    else if (
                        pos.x > 0 &&
                        pos.x + (!isSpace(*j) ? x : 0) >= maxLineWidth)
                    {
                        if (textLine != utf32.end())
                        {
                            j = textLine;
                            textLine = utf32.end();
                            size.x = std::max(size.x, textLineX);
                            pos.x = 0;
                            pos.y += i->second->size->metrics.height / 64;
                        }
                        else
                        {
                            size.x = std::max(size.x, pos.x);
                            pos.x = x;
                            pos.y += i->second->size->metrics.height / 64;
                        }
                        rsbDeltaPrev = 0;
                    }
                    else
                    {
                        if (isSpace(*j) && j != utf32.begin())
                        {
                            textLine = j;
                            textLineX = pos.x;
                        }
                        pos.x += x;
                    }
                }
                size.x = std::max(size.x, pos.x);
                size.y = pos.y;
            }
        }
    }
}
