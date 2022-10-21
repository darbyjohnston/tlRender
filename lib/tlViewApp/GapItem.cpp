// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2022 Darby Johnston
// All rights reserved.

#include <tlViewApp/GapItem.h>

#include <tlViewApp/Util.h>

#include <tlCore/StringFormat.h>

namespace tl
{
    namespace view
    {
        struct GapItem::Private
        {
        };

        void GapItem::_init(
            otio::Gap* gap,
            const std::shared_ptr<IGraphicsItem>& parent)
        {
            IGraphicsItem::_init(parent);

            TLRENDER_P();

            _type = "Gap";
            _name = gap->name();
            _duration = gap->duration();
        }

        GapItem::GapItem() :
            _p(new Private)
        {}

        GapItem::~GapItem()
        {}

        std::shared_ptr<GapItem> GapItem::create(
            otio::Gap* gap,
            const std::shared_ptr<IGraphicsItem>&parent)
        {
            auto out = std::shared_ptr<GapItem>(new GapItem);
            out->_init(gap, parent);
            return out;
        }

        math::Vector2i GapItem::getSize(
            const std::shared_ptr<imaging::FontSystem>& fontSystem) const
        {
            imaging::FontMetrics fontMetrics = fontSystem->getMetrics(itemTitleFontInfo);
            math::Vector2i size = math::Vector2i(
                _duration.rescaled_to(1.0).value() * secondsSize,
                fontMetrics.lineHeight + itemMargin * 2 + itemBorder * 2);
            return size;
        }

        void GapItem::draw(
            const math::BBox2i & bbox,
            const std::shared_ptr<imaging::FontSystem>& fontSystem,
            const std::shared_ptr<timeline::IRender>&render)
        {
            render->drawRect(bbox, imaging::Color4f(.8F, .8F, .8F));

            const math::BBox2i marginRect = bbox.margin(-(itemMargin + itemBorder));

            math::BBox2i textRect = marginRect;
            imaging::FontMetrics fontMetrics = fontSystem->getMetrics(itemSmallFontInfo);
            math::Vector2i textSize = fontSystem->measure(_type, itemSmallFontInfo);
            auto textGlyphs = fontSystem->getGlyphs(_type, itemSmallFontInfo);
            math::Vector2i textPos(
                textRect.min.x,
                textRect.min.y + fontMetrics.ascender - 1);
            render->drawText(textGlyphs, textPos, imaging::Color4f(0.F, 0.F, 0.F));

            fontMetrics = fontSystem->getMetrics(itemTitleFontInfo);
            textSize = fontSystem->measure(_name, itemTitleFontInfo);
            textGlyphs = fontSystem->getGlyphs(_name, itemTitleFontInfo);
            textPos = math::Vector2i(
                textRect.min.x + textRect.w() / 2 - textSize.x / 2,
                textRect.min.y + textRect.h() / 2 - textSize.y / 2 + fontMetrics.ascender - 1);
            render->drawText(textGlyphs, textPos, imaging::Color4f(0.F, 0.F, 0.F));
        }
    }
}
