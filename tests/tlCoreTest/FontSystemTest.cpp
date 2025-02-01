// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2025 Darby Johnston
// All rights reserved.

#include <tlCoreTest/FontSystemTest.h>

#include <tlCore/FontSystem.h>

#include <dtk/core/Format.h>

using namespace tl::image;

namespace tl
{
    namespace core_tests
    {
        FontSystemTest::FontSystemTest(const std::shared_ptr<system::Context>& context) :
            ITest("core_tests::FontSystemTest", context)
        {}

        std::shared_ptr<FontSystemTest> FontSystemTest::create(const std::shared_ptr<system::Context>& context)
        {
            return std::shared_ptr<FontSystemTest>(new FontSystemTest(context));
        }

        void FontSystemTest::run()
        {
            for (const auto& font :
                {
                    "NotoMono-Regular",
                    "NotoSans-Regular",
                    "NotoSans-Bold"
                })
            {
                auto data = getFontData(font);
                DTK_ASSERT(!data.empty());
            }
            {
                const FontInfo fi;
                DTK_ASSERT("NotoSans-Regular" == fi.family);
                DTK_ASSERT(12 == fi.size);
            }
            {
                const FontInfo fi("NotoMono-Regular", 14);
                DTK_ASSERT("NotoMono-Regular" == fi.family);
                DTK_ASSERT(14 == fi.size);
            }
            {
                FontInfo a;
                FontInfo b;
                DTK_ASSERT(a == b);
            }
            {
                FontInfo a("NotoMono-Regular", 14);
                FontInfo b;
                DTK_ASSERT(a < b);
            }
            {
                const GlyphInfo gi;
                DTK_ASSERT(0 == gi.code);
                DTK_ASSERT(FontInfo() == gi.fontInfo);
            }
            {
                const FontInfo fi("NotoMono-Regular", 14);
                const GlyphInfo gi(1, fi);
                DTK_ASSERT(1 == gi.code);
                DTK_ASSERT(fi == gi.fontInfo);
            }
            {
                GlyphInfo a;
                GlyphInfo b;
                DTK_ASSERT(a == b);
            }
            {
                GlyphInfo a;
                GlyphInfo b(1, FontInfo("NotoMono-Regular", 14));
                DTK_ASSERT(a < b);
            }
            auto fontSystem = _context->getSystem<image::FontSystem>();
            for (auto fontSize : { 14, 0 })
            {
                _print(dtk::Format("Font size: {0}").arg(fontSize));
                FontInfo fi("NotoMono-Regular", fontSize);
                auto fm = fontSystem->getMetrics(fi);
                std::vector<std::string> text =
                {
                    "Hello world!",
                    "Hello\nworld!",
                    "Hello world!"
                };
                std::vector<uint16_t> maxLineWidth =
                {
                    0,
                    0,
                    1
                };
                for (size_t i = 0; i < text.size(); ++i)
                {
                    _print(dtk::Format("Text: {0}").arg(text[i]));
                    const math::Size2i size = fontSystem->getSize(text[i], fi, maxLineWidth[i]);
                    _print(dtk::Format("Size: {0}").arg(size));
                    const auto boxes = fontSystem->getBox(text[i], fi, maxLineWidth[i]);
                    DTK_ASSERT(text[i].size() == boxes.size());
                    for (size_t j = 0; j < text[i].size(); ++j)
                    {
                        _print(dtk::Format("Box '{0}': {1}").
                            arg(text[i][j]).
                            arg(boxes[j]));
                    }
                    const auto glyphs = fontSystem->getGlyphs(text[i], fi);
                    DTK_ASSERT(text[i].size() == glyphs.size());
                    for (size_t j = 0; j < text[i].size(); ++j)
                    {
                        image::Size size;
                        if (glyphs[j] && glyphs[j]->image)
                        {
                            size = glyphs[j]->image->getSize();
                        }
                        _print(dtk::Format("Glyph '{0}': {1}").
                            arg(text[i][j]).
                            arg(size));
                    }
                    _print(dtk::Format("Glyph cache size: {0}").
                        arg(fontSystem->getGlyphCacheSize()));
                    _print(dtk::Format("Glyph cache percentage: {0}%").
                        arg(fontSystem->getGlyphCachePercentage()));
                }
            }
        }
    }
}
