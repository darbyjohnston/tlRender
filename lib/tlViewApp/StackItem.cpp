// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2022 Darby Johnston
// All rights reserved.

#include <tlViewApp/StackItem.h>

#include <tlViewApp/Util.h>

#include <tlCore/StringFormat.h>

namespace tl
{
    namespace view
    {
        struct StackItem::Private
        {
        };

        void StackItem::_init(
            otio::Stack* stack,
            const std::shared_ptr<IGraphicsItem>& parent)
        {
            IGraphicsItem::_init(parent);

            TLRENDER_P();

            _type = "Stack";
            _name = stack->name();
            _duration = stack->duration();
            _trimmedRange = string::Format("Trimmed range: {0}").arg(stack->trimmed_range());
            const auto& sourceRange = stack->source_range();
            std::stringstream ss;
            if (sourceRange.has_value())
            {
                ss << sourceRange.value();
            }
            else
            {
                ss << "none";
            }
            _sourceRange = string::Format("Source range: {0}").arg(ss.str());
        }

        StackItem::StackItem() :
            _p(new Private)
        {}

        StackItem::~StackItem()
        {}

        std::shared_ptr<StackItem> StackItem::create(
            otio::Stack* stack,
            const std::shared_ptr<IGraphicsItem>&parent)
        {
            auto out = std::shared_ptr<StackItem>(new StackItem);
            out->_init(stack,  parent);
            return out;
        }

        math::Vector2i StackItem::getSize(
            const std::shared_ptr<imaging::FontSystem>& fontSystem) const
        {
            const imaging::FontMetrics titleFontMetrics = fontSystem->getMetrics(itemTitleFontInfo);
            const imaging::FontMetrics smallFontMetrics = fontSystem->getMetrics(itemSmallFontInfo);
            math::Vector2i size = math::Vector2i(
                _duration.rescaled_to(1.0).value() * secondsSize,
                smallFontMetrics.lineHeight * 2 + titleFontMetrics.lineHeight + itemMargin * 2 + itemBorder * 2);
            return size;
        }

        void StackItem::draw(
            const math::BBox2i& bbox,
            const std::shared_ptr<imaging::FontSystem>& fontSystem,
            const std::shared_ptr<timeline::IRender>&render)
        {
            const imaging::FontMetrics titleFontMetrics = fontSystem->getMetrics(itemTitleFontInfo);
            const imaging::FontMetrics smallFontMetrics = fontSystem->getMetrics(itemSmallFontInfo);

            math::Vector2i textSize = fontSystem->measure(_trimmedRange, itemSmallFontInfo);
            auto textGlyphs = fontSystem->getGlyphs(_trimmedRange, itemSmallFontInfo);
            math::Vector2i textPos = math::Vector2i(bbox.min.x, bbox.min.y + smallFontMetrics.ascender - 1);
            render->drawText(textGlyphs, textPos, imaging::Color4f(0.F, 0.F, 0.F));

            textSize = fontSystem->measure(_sourceRange, itemSmallFontInfo);
            textGlyphs = fontSystem->getGlyphs(_sourceRange, itemSmallFontInfo);
            textPos = math::Vector2i(bbox.min.x, bbox.max.y - textSize.y + smallFontMetrics.ascender - 1);
            render->drawText(textGlyphs, textPos, imaging::Color4f(0.F, 0.F, 0.F));

            const math::BBox2i rect = bbox.margin(0, -smallFontMetrics.lineHeight, 0, -smallFontMetrics.lineHeight);

            render->drawRect(rect, imaging::Color4f(.8F, .8F, .8F));

            const math::BBox2i marginRect = rect.margin(-(itemMargin + itemBorder));

            textSize = fontSystem->measure(_type, itemSmallFontInfo);
            textGlyphs = fontSystem->getGlyphs(_type, itemSmallFontInfo);
            textPos = math::Vector2i(
                marginRect.min.x,
                marginRect.min.y + smallFontMetrics.ascender - 1);
            render->drawText(textGlyphs, textPos, imaging::Color4f(0.F, 0.F, 0.F));

            textSize = fontSystem->measure(_name, itemTitleFontInfo);
            textGlyphs = fontSystem->getGlyphs(_name, itemTitleFontInfo);
            textPos = math::Vector2i(
                marginRect.min.x + marginRect.w() / 2 - textSize.x / 2,
                marginRect.min.y + marginRect.h() / 2 - textSize.y / 2 + titleFontMetrics.ascender - 1);
            render->drawText(textGlyphs, textPos, imaging::Color4f(0.F, 0.F, 0.F));
        }
    }
}
