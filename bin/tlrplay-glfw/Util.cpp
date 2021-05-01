// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021 Darby Johnston
// All rights reserved.

#include "Util.h"

#include <tlrRender/Math.h>

#include <cmath>

namespace tlr
{
    math::BBox2f fitImageInWindow(const imaging::Size& image, const imaging::Size& window)
    {
        math::BBox2f out;
        const float windowAspect = window.getAspect();
        const float imageAspect = image.getAspect();
        math::BBox2f bbox;
        if (windowAspect > imageAspect)
        {
            out = math::BBox2f(
                window.w / 2.F - (window.h * imageAspect) / 2.F,
                0.F,
                window.h * imageAspect,
                window.h);
        }
        else
        {
            out = math::BBox2f(
                0.F,
                window.h / 2.F - (window.w / imageAspect) / 2.F,
                window.w,
                window.w / imageAspect);
        }
        return out;
    }

    void drawHUDLabel(
        const std::shared_ptr<Render>& render,
        const std::shared_ptr<FontSystem>& fontSystem,
        const imaging::Size& window,
        const std::string& text,
        FontFamily fontFamily,
        uint16_t fontSize,
        HUDElement hudElement)
    {
        const imaging::Color4f labelColor(1.F, 1.F, 1.F);
        const imaging::Color4f overlayColor(0.F, 0.F, 0.F, .7F);

        const FontInfo fontInfo(fontFamily, fontSize);
        const FontMetrics fontMetrics = fontSystem->getMetrics(fontInfo);

        const float margin = fontSize;
        const math::BBox2f marginBBox = math::BBox2f(0.F, 0.F, window.w, window.h).margin(-margin);
        const math::Vector2f labelSize = fontSystem->measure(text, fontInfo);
        const float labelMargin = fontSize / 5.F;
        math::BBox2f bbox;
        math::Vector2f pos;
        switch (hudElement)
        {
        case HUDElement::UpperLeft:
            bbox = math::BBox2f(
                floorf(marginBBox.min.x),
                floorf(marginBBox.min.y),
                ceilf(labelSize.x + labelMargin * 2.F),
                ceilf(fontMetrics.lineHeight + labelMargin * 2.F));
            pos = math::Vector2f(
                floorf(marginBBox.min.x + labelMargin),
                floorf(marginBBox.min.y + labelMargin + fontMetrics.ascender));
            break;
        case HUDElement::UpperRight:
            bbox = math::BBox2f(
                floorf(marginBBox.max.x - labelMargin * 2.F - labelSize.x),
                floorf(marginBBox.min.y),
                ceilf(labelSize.x + labelMargin * 2.F),
                ceilf(fontMetrics.lineHeight + labelMargin * 2.F));
            pos = math::Vector2f(
                floorf(marginBBox.max.x - labelMargin - labelSize.x),
                floorf(marginBBox.min.y + labelMargin + fontMetrics.ascender));
            break;
        case HUDElement::LowerLeft:
            bbox = math::BBox2f(
                floorf(marginBBox.min.x),
                floorf(marginBBox.max.y - labelMargin * 2.F - fontMetrics.lineHeight),
                ceilf(labelSize.x + labelMargin * 2.F),
                ceilf(fontMetrics.lineHeight + labelMargin * 2.F));
            pos = math::Vector2f(
                floorf(marginBBox.min.x + labelMargin),
                floorf(marginBBox.max.y - labelMargin - fontMetrics.lineHeight + fontMetrics.ascender));
            break;
        case HUDElement::LowerRight:
            bbox = math::BBox2f(
                floorf(marginBBox.max.x - labelMargin * 2.F - labelSize.x),
                floorf(marginBBox.max.y - labelMargin * 2.F - fontMetrics.lineHeight),
                ceilf(labelSize.x + labelMargin * 2.F),
                ceilf(fontMetrics.lineHeight + labelMargin * 2.F));
            pos = math::Vector2f(
                floorf(marginBBox.max.x - labelMargin - labelSize.x),
                floorf(marginBBox.max.y - labelMargin - fontMetrics.lineHeight + fontMetrics.ascender));
            break;
        }

        render->drawRect(bbox, overlayColor);
        render->drawText(fontSystem->getGlyphs(text, fontInfo), pos, labelColor);
    }
}
